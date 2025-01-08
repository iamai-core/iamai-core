#include "interface.h"
#include <iostream>

Interface::Interface(const std::string& modelPath) {
    // Load all available backends
    ggml_backend_load_all();

    // Initialize model parameters
    auto model_params = llama_model_default_params();
    // model_params.n_gpu_layers = 100;
    model = llama_load_model_from_file(modelPath.c_str(), model_params);

    if (model == NULL) {
        fprintf(stderr, "error: failed to load model from '%s'\n", modelPath.c_str());
        throw std::runtime_error("Failed to load model");
    }

    // Initialize context parameters
    auto ctx_params = llama_context_default_params();
    ctx_params.n_ctx = n_ctx;
    ctx_params.n_batch = n_batch;
    ctx_params.n_threads = n_threads;
    ctx = llama_new_context_with_model(model, ctx_params);

    if (ctx == NULL) {
        fprintf(stderr, "error: failed to create context\n");
        llama_free_model(model);
        throw std::runtime_error("Failed to create context");
    }

    // Initialize sampler
    auto sparams = llama_sampler_chain_default_params();
    sampler = llama_sampler_chain_init(sparams);

    // Add default sampling settings
    llama_sampler_chain_add(sampler, llama_sampler_init_top_k(40));
    llama_sampler_chain_add(sampler, llama_sampler_init_top_p(0.95f, 1));
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(0.8f));
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
}

Interface::~Interface() {
    if (sampler != NULL) {
        llama_sampler_free(sampler);
    }
    if (ctx != NULL) {
        llama_free(ctx);
    }
    if (model != NULL) {
        llama_free_model(model);
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
    int n_chars = llama_token_to_piece(model, new_token_id, buf, sizeof(buf), 0, true);
    if (n_chars < 0) {
        throw std::runtime_error("Failed to convert token to text");
    }

    // Check if we hit the end of sequence token
    if (llama_token_is_eog(model, new_token_id)) {
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
    // Tokenize the prompt
    int n_prompt_tokens = -llama_tokenize(model, prompt.c_str(), prompt.length(), NULL, 0, true, false);
    std::vector<llama_token> tokens(n_prompt_tokens);

    if (llama_tokenize(model, prompt.c_str(), prompt.length(), tokens.data(), tokens.size(), true, false) < 0) {
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

    return result;
}
