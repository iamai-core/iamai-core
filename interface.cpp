#include "interface.h"
#include <iostream>

Interface::Interface(const std::string& modelPath) {
    // Load all available backends
    ggml_backend_load_all();

    // Initialize model parameters
    auto model_params = llama_model_default_params();
    model = llama_load_model_from_file(modelPath.c_str(), model_params);

    if (model == NULL) {
        fprintf(stderr, "error: failed to load model from '%s'\n", modelPath.c_str());
        throw std::runtime_error("Failed to load model");
    }

    // Initialize context parameters
    auto ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 512;  // Set reasonable default context size
    ctx = llama_new_context_with_model(model, ctx_params);

    if (ctx == NULL) {
        fprintf(stderr, "error: failed to create context\n");
        llama_free_model(model);
        throw std::runtime_error("Failed to create context");
    }
}

Interface::~Interface() {
    if (ctx != NULL) {
        llama_free(ctx);
    }
    if (model != NULL) {
        llama_free_model(model);
    }
}

void Interface::share(const std::string& text) {
    // Get required token count (returns negative of required size)
    int n_tokens = llama_tokenize(model, text.c_str(), text.length(), NULL, 0, true, false);
    n_tokens = -n_tokens; // Convert to positive count

    // Allocate vector and tokenize
    tokens.resize(n_tokens);
    if (llama_tokenize(model, text.c_str(), text.length(), tokens.data(), tokens.size(), true, false) < 0) {
        throw std::runtime_error("Tokenization failed");
    }
}

std::string Interface::collect() {
    if (tokens.empty()) {
        return "";
    }

    std::string result;
    for (const auto& token : tokens) {
        char buf[8] = {0};
        int n_chars = llama_token_to_piece(model, token, buf, sizeof(buf), 0, true);
        if (n_chars < 0) {
            throw std::runtime_error("Failed to convert token to text");
        }
        result.append(buf, n_chars);
    }

    return result;
}
