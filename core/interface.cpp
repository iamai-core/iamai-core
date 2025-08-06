#include "interface.h"
#include "ggml-backend.h"
#include <iostream>
#include <thread>
#include <algorithm>
#define EXPORT _declspec(dllexport)

extern "C" {
    // C-style factory functions
    EXPORT Interface* create_interface(const char* model_path) {
        try {
            return new Interface(model_path);
        } catch (...) {
            return nullptr;
        }
    }

    EXPORT void free_interface(Interface* ptr) {
        delete ptr;
    }

    // Wrapper for generate function
    EXPORT const char* interface_generate(Interface* ptr, const char* prompt) {
        try {
            static std::string result;  // Keep buffer alive for ctypes
            result = ptr->generate(prompt);
            return result.c_str();
        } catch (...) {
            return nullptr;
        }
    }
}

Interface::Interface(const std::string& modelPath) {
    // Load model first to auto-detect optimal settings
    loadModel(modelPath);

    // Get the model's training context size and set optimal defaults
    int n_ctx_train = llama_model_n_ctx_train(model);
    config.ctx = n_ctx_train;
    config.batch = n_ctx_train;  // Set batch to match context for maximum efficiency

    // Set thread defaults
    setThreadDefaults();

    // Complete initialization
    initializeContext();
}

Interface::Interface(const std::string& modelPath, Config config) {
    this->config = config;
    loadModel(modelPath);
    initializeContext();
}

void Interface::loadModel(const std::string& modelPath) {
    ggml_backend_load_all();

    auto model_params = llama_model_default_params();
    model_params.n_gpu_layers = 0;

    model = llama_model_load_from_file(modelPath.c_str(), model_params);
    if (model == NULL) {
        throw std::runtime_error("Failed to load model");
    }

    vocab = llama_model_get_vocab(model);
}

void Interface::setThreadDefaults() {
    unsigned int maxThreads = std::thread::hardware_concurrency();
    if (maxThreads <= 4) config.threads = 1;
    else config.threads = maxThreads / 4;
}

void Interface::initializeModel(const std::string& modelPath) {
    // This method is kept for backward compatibility if needed
    // but now just calls the refactored methods
    loadModel(modelPath);
}

void Interface::initializeContext() {
    auto ctx_params = llama_context_default_params();
    ctx_params.n_ctx = config.ctx;
    ctx_params.n_batch = config.batch;
    ctx_params.n_threads = config.threads;
    ctx_params.n_threads_batch = config.threads;

    ctx = llama_init_from_model(model, ctx_params);
    if (ctx == NULL) {
        llama_model_free(model);
        throw std::runtime_error("Failed to create context");
    }

    // Get memory handle for KV cache management
    memory = llama_get_memory(ctx);
    if (memory == NULL) {
        throw std::runtime_error("Failed to get memory handle");
    }

    auto sparams = llama_sampler_chain_default_params();
    sampler = llama_sampler_chain_init(sparams);

    llama_sampler_chain_add(sampler, llama_sampler_init_top_k(config.top_k));
    llama_sampler_chain_add(sampler, llama_sampler_init_top_p(config.top_p, 1));
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(config.temperature));
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(config.seed));

    // Initialize context state
    n_past = 0;
    token_history.clear();
}

Interface::~Interface() {
    if (sampler != NULL) {
        llama_sampler_free(sampler);
    }
    if (ctx != NULL) {
        llama_free(ctx);
    }
    if (model != NULL) {
        llama_model_free(model);
    }
}

std::vector<llama_token> Interface::tokenize(const std::string& text, bool add_bos) {
    int n_tokens = -llama_tokenize(vocab, text.c_str(), text.length(), NULL, 0, add_bos, false);
    std::vector<llama_token> tokens(n_tokens);

    if (llama_tokenize(vocab, text.c_str(), text.length(), tokens.data(), tokens.size(), add_bos, false) < 0) {
        throw std::runtime_error("Tokenization failed");
    }

    return tokens;
}

bool Interface::canFitTokens(int num_tokens) {
    return (n_past + num_tokens) <= config.ctx;
}

void Interface::manageContext(const std::vector<llama_token>& new_tokens) {
    int tokens_needed = new_tokens.size() + config.max_tokens; // Reserve space for generation

    if (!canFitTokens(tokens_needed)) {
        // Calculate how many tokens to remove
        int total_needed = n_past + tokens_needed;
        int overflow = total_needed - config.ctx;

        // Remove at least the overflow, but consider keeping more context
        int tokens_to_remove = std::max(overflow,
            static_cast<int>(n_past * (1.0f - config.cache_keep_ratio)));

        // Ensure we don't remove too much
        tokens_to_remove = std::min(tokens_to_remove, n_past - config.min_keep_tokens);
        tokens_to_remove = std::max(tokens_to_remove, 0);

        if (tokens_to_remove > 0) {
            shiftContext(tokens_to_remove);
        }
    }
}

void Interface::shiftContext(int tokens_to_remove) {
    if (tokens_to_remove <= 0 || tokens_to_remove >= n_past) {
        return;
    }

    std::cout << "Shifting context: removing " << tokens_to_remove
              << " tokens from " << n_past << " total" << std::endl;

    // Remove tokens from the beginning of the sequence
    llama_memory_seq_rm(memory, MAIN_SEQ, 0, tokens_to_remove);

    // Update our tracking
    n_past -= tokens_to_remove;

    // Remove corresponding tokens from history
    for (int i = 0; i < tokens_to_remove && !token_history.empty(); i++) {
        token_history.pop_front();
    }

    // Note: RoPE positions are automatically handled by llama.cpp
    // The remaining tokens will have their positions adjusted
}

void Interface::evaluateTokens(const std::vector<llama_token>& tokens) {
    if (tokens.empty()) return;

    // Create batch for evaluation
    llama_batch batch = llama_batch_get_one(
        const_cast<llama_token*>(tokens.data()),
        tokens.size()
    );

    if (llama_decode(ctx, batch)) {
        throw std::runtime_error("Failed to evaluate tokens");
    }

    // Update position and history
    n_past += tokens.size();
    for (const auto& token : tokens) {
        token_history.push_back(token);
    }
}

std::string Interface::sampleTokens(bool& should_stop) {
    std::string result;

    llama_token new_token_id = llama_sampler_sample(sampler, ctx, -1);
    llama_sampler_accept(sampler, new_token_id);

    char buf[128];
    int n_chars = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
    if (n_chars < 0) {
        throw std::runtime_error("Failed to convert token to text");
    }

    if (llama_vocab_is_eog(vocab, new_token_id)) {
        should_stop = true;
        return result;
    }

    result.append(buf, n_chars);

    // Evaluate the new token
    std::vector<llama_token> new_token = {new_token_id};
    evaluateTokens(new_token);

    return result;
}

void Interface::setPromptFormat(const std::string& promptFormat) {
    if (promptFormat.empty()) {
        clearPromptFormat();
        return;
    }
    formatPrompt = true;
    this->promptFormat = promptFormat;
}

void Interface::clearPromptFormat() {
    formatPrompt = false;
    promptFormat = "";
}

void Interface::clearContext() {
    llama_memory_clear(memory, true);
    n_past = 0;
    token_history.clear();
    llama_sampler_reset(sampler);
}

int Interface::getContextUsage() {
    return n_past;
}

int Interface::getContextSize() {
    return config.ctx;
}

void Interface::formatNewPrompt(const std::string& input, std::string& output) {
    output = promptFormat;
    const std::string placeholder = "{prompt}";
    size_t pos = output.find(placeholder);
    if (pos != std::string::npos) {
        output.replace(pos, placeholder.length(), input);
    }
}

std::string Interface::generate(const std::string& prompt) {
    std::string formattedPrompt = prompt;
    if (formatPrompt) {
        formatNewPrompt(prompt, formattedPrompt);
    }

    // Tokenize the new prompt
    std::vector<llama_token> new_tokens = tokenize(formattedPrompt, n_past == 0);

    // Manage context to make room for new tokens + generation
    manageContext(new_tokens);

    // Evaluate the new prompt tokens
    evaluateTokens(new_tokens);

    // Generate response
    std::string result;
    bool should_stop = false;

    for (int i = 0; i < config.max_tokens && !should_stop; i++) {
        // Check if we're approaching context limit during generation
        if (n_past >= config.ctx - 10) {
            std::cout << "Warning: Approaching context limit during generation" << std::endl;
            break;
        }

        std::string token_str = sampleTokens(should_stop);
        result += token_str;
    }

    return result;
}
