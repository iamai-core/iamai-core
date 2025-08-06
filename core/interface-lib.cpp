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

EXPORT Context* Init(const char* model_path) {
    try {
        Context* ctx = new Context();
        ctx->interface = new Interface(model_path);
        return ctx;
    } catch (...) {
        return nullptr;
    }
}

EXPORT Context* FullInit(const char* model_path, int max_tokens, int batch, int size, int threads, int top_k, float top_p, float temperature, uint32_t seed) {
    try {
        Context* ctx = new Context();

        Interface::Config config;
        config.max_tokens = max_tokens;
        config.batch = batch;
        config.ctx = size;
        config.threads = threads;

        config.seed = seed;
        config.temperature = temperature;
        config.top_k = top_k;
        config.top_p = top_p;

        ctx->interface = new Interface(model_path, config);
        return ctx;
    } catch (...) {
        return nullptr;
    }
}

EXPORT void SetPromptFormat(Context* ctx, const char* format) {
    if (ctx) ctx->interface->setPromptFormat(format);
}

EXPORT void ClearPromptFormat(Context* ctx) {
    if (ctx) ctx->interface->clearPromptFormat();
}

// New KV cache management functions
EXPORT void ClearContext(Context* ctx) {
    if (ctx) ctx->interface->clearContext();
}

EXPORT int GetContextUsage(Context* ctx) {
    if (ctx) return ctx->interface->getContextUsage();
    return 0;
}

EXPORT int GetContextSize(Context* ctx) {
    if (ctx) return ctx->interface->getContextSize();
    return 0;
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

// Cleanup
EXPORT void Free(Context* ctx) {
    if (ctx) {
        delete ctx->interface;
        delete ctx;
    }
}

}
