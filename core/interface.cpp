#include "interface.h"
#include "ggml-backend.h"
#include <iostream>
#include <thread>
#include <algorithm>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

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
    setThreadDefaults();

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

    // Check if model has a chat template
    const char* template_str = llama_model_chat_template(model, nullptr);
    hasTemplate = (template_str != nullptr);
    if (hasTemplate) chatTemplate = template_str;

    // Extract role marker from template and tokenize it
    if (hasTemplate) {
        std::string stop_string;

        // Find the pattern for starting a new role
        if (chatTemplate.find("<|start_header_id|>") != std::string::npos) {
            stop_string = "<|start_header_id|>";
        } else if (chatTemplate.find("<|im_start|>") != std::string::npos) {
            stop_string = "<|im_start|>";
        } else if (chatTemplate.find("<start_of_turn>") != std::string::npos) {
            stop_string = "<start_of_turn>";
        } else if (chatTemplate.find("<|user|>") != std::string::npos) {
            stop_string = "<|user|>";
        } else if (chatTemplate.find("<|assistant|>") != std::string::npos) {
            stop_string = "<|user|>";
        } else if (chatTemplate.find("<｜User｜>") != std::string::npos) {
            stop_string = "<｜User｜>";
        }

        // Tokenize the stop string to get its token ID
        if (!stop_string.empty()) {
            auto stop_tokens = tokenize(stop_string, false, true);  // parse_special=true
            if (!stop_tokens.empty()) {
                stop_token = stop_tokens[0];  // Usually a single token
            }
        }
    }
}

void Interface::setThreadDefaults() {
    unsigned int maxThreads = std::thread::hardware_concurrency();
    if (maxThreads > 0) config.threads = maxThreads;
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

    // Initialize sampler chain
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

std::vector<llama_token> Interface::tokenize(const std::string& text, bool add_bos, bool parse_special) {
    int n_tokens = -llama_tokenize(vocab, text.c_str(), text.length(), NULL, 0, add_bos, parse_special);
    std::vector<llama_token> tokens(n_tokens);

    if (llama_tokenize(vocab, text.c_str(), text.length(), tokens.data(), tokens.size(), add_bos, parse_special) < 0) {
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

    // Remove tokens from the beginning of the sequence using the memory API
    llama_memory_seq_rm(memory, MAIN_SEQ, 0, tokens_to_remove);

    // Update position tracking
    n_past -= tokens_to_remove;

    // Remove corresponding tokens from history
    for (int i = 0; i < tokens_to_remove && !token_history.empty(); i++) {
        token_history.pop_front();
    }
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

std::string Interface::sampleTokens(bool& should_stop, bool is_first) {
    std::string result;

    llama_token new_token_id = llama_sampler_sample(sampler, ctx, -1);
    llama_sampler_accept(sampler, new_token_id);

    // Check for EOG tokens
    if (llama_vocab_is_eog(vocab, new_token_id)) {
        should_stop = true;
        return result;
    }

    // Check for role marker token (if using chat template)
    if (formatPrompt && hasTemplate &&
        stop_token != LLAMA_TOKEN_NULL &&
        new_token_id == stop_token) {
        should_stop = true;
        return result;
    }

    char buf[128];
    int lstrip = is_first ? 1 : 0;
    int n_chars = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), lstrip, true);
    if (n_chars < 0) {
        throw std::runtime_error("Failed to convert token to text");
    }

    result.append(buf, n_chars);

    // Evaluate the new token
    std::vector<llama_token> new_token = {new_token_id};
    evaluateTokens(new_token);

    return result;
}

void Interface::setPromptFormat(const std::string& promptFormat) {
    if (hasTemplate) {
        formatPrompt = true;
    }
}

void Interface::clearPromptFormat() {
    formatPrompt = false;
}

void Interface::clearContext() {
    llama_memory_clear(memory, true);
    n_past = 0;
    token_history.clear();
    llama_sampler_reset(sampler);
}

int Interface::getContextUsage() {
    // Use the memory API to get actual usage
    llama_pos pos = llama_memory_seq_pos_max(memory, MAIN_SEQ);
    return pos >= 0 ? pos + 1 : 0;
}

int Interface::getContextSize() {
    return config.ctx;
}

std::string Interface::applyChatTemplate(const std::string& userMessage) {
    // Create a single message for the current user input
    llama_chat_message msg = {"user", userMessage.c_str()};

    // Apply template to just this one message
    std::vector<char> formatted(config.ctx);
    int new_len = llama_chat_apply_template(
        chatTemplate.c_str(),  // Use the model's chat template
        &msg,
        1,        // Just one message
        true,     // Add assistant token
        formatted.data(),
        formatted.size()
    );

    if (new_len < 0) {
        throw std::runtime_error("Failed to apply chat template");
    }

    return std::string(formatted.begin(), formatted.begin() + new_len);
}

std::string Interface::generate(const std::string& prompt) {
    // Check if we should use chat template formatting
    bool use_chat_template = formatPrompt && hasTemplate;

    std::string formattedPrompt;
    if (use_chat_template) {
        formattedPrompt = applyChatTemplate(prompt);
    } else {
        formattedPrompt = prompt;
    }

    // Tokenize the new prompt (parse special tokens when using chat templates)
    std::vector<llama_token> new_tokens = tokenize(formattedPrompt, n_past == 0, use_chat_template);

    // Manage context to make room for new tokens + generation
    manageContext(new_tokens);

    // Evaluate the new prompt tokens
    evaluateTokens(new_tokens);

    // Generate response
    std::string result;
    bool should_stop = false;

    for (int i = 0; i < config.max_tokens && !should_stop; i++) {
        // Check if we're approaching context limit during generation
        if (n_past >= config.ctx - 2) {
            std::cout << "Warning: Approaching context limit during generation" << std::endl;
            break;
        }

        std::string token_str = sampleTokens(should_stop, i == 0);
        result += token_str;
    }

    return result;
}
