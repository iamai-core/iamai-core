#include "interface.h"
#include "ggml-backend.h"
#include <iostream>
#include <thread>
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

    unsigned int maxThreads = std::thread::hardware_concurrency();
    if (maxThreads <= 4) config.n_threads = 1;
    else config.n_threads = maxThreads / 4;

    if (config.n_threads >= 16) config.n_batch = 64;
    else if (config.n_threads >= 8) config.n_batch = 32;
    else config.n_batch = 16;

    initializeModel(modelPath);
    
}

Interface::Interface(const std::string& modelPath, Config config) {
    
    this->config = config;
    initializeModel(modelPath);
    
}

void Interface::initializeModel(const std::string& modelPath) {

    // Load all available backends
    ggml_backend_load_all();

    // Initialize model parameters without CUDA
    auto model_params = llama_model_default_params();

    // No GPU layers - run on CPU only
    model_params.n_gpu_layers = 0;  // Set to 0 to disable GPU usage

    fprintf(stderr, "Loading model on CPU only (n_gpu_layers = %d)...\n",
            model_params.n_gpu_layers);

    // Load the model
    model = llama_model_load_from_file(modelPath.c_str(), model_params);
    if (model == NULL) {
        
        throw std::runtime_error("Failed to load model");
        
    }

    // Get vocab handle
    vocab = llama_model_get_vocab(model);

    // Initialize context parameters for CPU optimization
        auto ctx_params = llama_context_default_params();
        ctx_params.n_ctx = config.n_ctx;
        ctx_params.n_batch = config.n_batch;  // Use the default batch size
        ctx_params.n_threads = config.n_threads;  // Use more threads for CPU
        ctx_params.n_threads_batch = config.n_threads;  // Match batch processing threads

        // Create context
        ctx = llama_init_from_model(model, ctx_params);
        if (ctx == NULL) {

            llama_model_free(model);
            throw std::runtime_error("Failed to create context");

        }

    // Initialize sampler with CPU-optimized parameters
    auto sparams = llama_sampler_chain_default_params();
    sampler = llama_sampler_chain_init(sparams);

    // Add sampling settings
    llama_sampler_chain_add(sampler, llama_sampler_init_top_k( config.top_k ));
    llama_sampler_chain_add(sampler, llama_sampler_init_top_p( config.top_p, 1));
    llama_sampler_chain_add(sampler, llama_sampler_init_temp( config.temperature ));
    llama_sampler_chain_add(sampler, llama_sampler_init_dist( config.seed ));

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

std::string Interface::sampleTokens(int& n_past, bool& should_stop) {
    std::string result;
    
    // Sample the next token
    llama_token new_token_id = llama_sampler_sample(sampler, ctx, -1);

    // Accept the token
    llama_sampler_accept(sampler, new_token_id);

    // Convert token to text
    char buf[128];
    int n_chars = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
    if (n_chars < 0) {
        throw std::runtime_error("Failed to convert token to text");
    }

    // Check if we hit the end of sequence token
    if (llama_vocab_is_eog(vocab, new_token_id)) {
        should_stop = true;
        return result;
    }

    // Add the generated piece to result
    result.append(buf, n_chars);

    // Prepare next batch with the sampled token
    llama_batch batch = llama_batch_get_one(&new_token_id, 1);
    if (llama_decode(ctx, batch)) {
        throw std::runtime_error("Failed to decode token");
    }
    n_past += 1;

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

void Interface::formatNewPrompt( const std::string& input, std::string& output ) {

    output = promptFormat;

    const std::string placeholder = "{prompt}";
    size_t pos = output.find(placeholder);

    if (pos != std::string::npos) output.replace(pos, placeholder.length(), input);

}


std::string Interface::generate(const std::string& prompt) {

    std::string formatedPrompt = prompt;
    if (formatPrompt) formatNewPrompt( prompt, formatedPrompt);

    // Tokenize the formatted prompt
    int n_prompt_tokens = -llama_tokenize(vocab, formatedPrompt.c_str(), formatedPrompt.length(), NULL, 0, true, false);
    std::vector<llama_token> tokens(n_prompt_tokens);

    if (llama_tokenize(vocab, formatedPrompt.c_str(), formatedPrompt.length(), tokens.data(), tokens.size(), true, false) < 0) {
        throw std::runtime_error("Tokenization failed");
    }

    // Process the prompt
    int n_past = 0;

    // Evaluate the prompt tokens
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    if (llama_decode(ctx, batch)) {
        throw std::runtime_error("failed to eval prompt");
    }
    n_past += batch.n_tokens;

    // Generate response
    std::string result;
    bool should_stop = false;

    for (int i = 0; i < config.max_tokens && !should_stop; i++) {
        
        std::string token_str = sampleTokens(n_past, should_stop);
        result += token_str;
        
    }

    return result;

}