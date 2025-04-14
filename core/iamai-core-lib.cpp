#include "interface.h"
#include <cstring>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

// Opaque pointer type
struct Context {
    Interface* interface;
};

extern "C" {

// Initialize the model
EXPORT Context* Init(const char* model_path) {
    try {
        Context* ctx = new Context();
        ctx->interface = new Interface(model_path);
        return ctx;
    } catch (...) {
        return nullptr;
    }
}

// Generate text from a prompt
EXPORT bool Generate(Context* ctx, const char* prompt, char* output, int output_size) {
    if (!ctx || !prompt || !output || output_size <= 0) {
        return false;
    }

    try {
        std::string result = ctx->interface->generate(prompt);
        strncpy(output, result.c_str(), output_size - 1);
        output[output_size - 1] = '\0';
        return true;
    } catch (...) {
        return false;
    }
}

// Configure model parameters
EXPORT void SetMaxTokens(Context* ctx, int max_tokens) {
    if (ctx) {
        ctx->interface->setMaxTokens(max_tokens);
    }
}

EXPORT void SetThreads(Context* ctx, int n_threads) {
    if (ctx) {
        ctx->interface->setThreads(n_threads);
    }
}

// Cleanup
EXPORT void Free(Context* ctx) {
    if (ctx) {
        delete ctx->interface;
        delete ctx;
    }
}

}
