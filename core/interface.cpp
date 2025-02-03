#include "interface.h"
#include "ggml-cuda.h"
#include "ggml-backend.h"
#include <iostream>

Interface::Interface(const std::string& modelPath) {
    // Load all available backends
    ggml_backend_load_all();

    // Initialize model parameters with CUDA support
    auto model_params = llama_model_default_params();

    // Force GPU usage - set to maximum to use all available layers
    model_params.n_gpu_layers = 999;  // This forces all possible layers to GPU

    fprintf(stderr, "Loading model with forced GPU layers (n_gpu_layers = %d)...\n",
            model_params.n_gpu_layers);

    // Load the model
    model = llama_model_load_from_file(modelPath.c_str(), model_params);
    if (model == NULL) {
        fprintf(stderr, "error: failed to load model from '%s'\n", modelPath.c_str());
        throw std::runtime_error("Failed to load model");
    }

    // Get vocab handle
    vocab = llama_model_get_vocab(model);

    // Initialize context parameters for GPU optimization
    auto ctx_params = llama_context_default_params();
    ctx_params.n_ctx = n_ctx;
    ctx_params.n_batch = 1024;  // Increased for GPU
    ctx_params.n_threads = 4;  // Reduced as we're using GPU
    ctx_params.n_threads_batch = 4;  // Batch processing threads

    fprintf(stderr, "Creating context with batch size %d...\n", ctx_params.n_batch);

    // Create context
    ctx = llama_init_from_model(model, ctx_params);
    if (ctx == NULL) {
        fprintf(stderr, "error: failed to create context\n");
        llama_model_free(model);
        throw std::runtime_error("Failed to create context");
    }

    // Initialize sampler with GPU-optimized parameters
    auto sparams = llama_sampler_chain_default_params();
    sampler = llama_sampler_chain_init(sparams);

    // Add sampling settings optimized for GPU
    llama_sampler_chain_add(sampler, llama_sampler_init_top_k(50));
    llama_sampler_chain_add(sampler, llama_sampler_init_top_p(0.9f, 1));
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(0.7f));
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
    

    fprintf(stderr, "Initialization complete\n");
}

Interface::~Interface() {
    if (sampler != NULL) {
        llama_sampler_free(sampler);
    }
    if (ctx != NULL) {
        llama_free(ctx);
    }
    if (model != NULL) {
        llama_model_free(model);  // Changed from llama_free_model
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

std::string Interface::generate(const std::string& prompt) {
    // Add DeepSeek-specific formatting
    const std::string formatted_prompt = 
        "<｜begin▁of▁sentence｜>You are a helpful AI assistant.\n\n"
        "User: " + prompt + "\n"
        "Assistant: ";

    // Tokenize the formatted prompt
    int n_prompt_tokens = -llama_tokenize(vocab, formatted_prompt.c_str(), 
                                        formatted_prompt.length(), NULL, 0, true, false);    
    std::vector<llama_token> tokens(n_prompt_tokens);

    if (llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens.data(), tokens.size(), true, false) < 0) {
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

    for (int i = 0; i < max_tokens && !should_stop; i++) {
        result += sampleTokens(n_past, should_stop);
    }

    return clean_response(result);
}

std::string Interface::clean_response(const std::string& response) {
    size_t end_pos = response.find("</s>");
    if(end_pos != std::string::npos) {
        return response.substr(0, end_pos);
    }
    
    end_pos = response.find("\nUser:");
    if(end_pos != std::string::npos) {
        return response.substr(0, end_pos);
    }
    
    return response;
}
