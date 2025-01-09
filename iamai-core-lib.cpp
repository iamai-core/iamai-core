#include "interface.h"
#include <cstring>

extern "C" {

// Opaque pointer for our Interface instance
struct IAMAIHandle {
    Interface* interface;
};

// Create a new instance
__declspec(dllexport) IAMAIHandle* iamai_create(const char* model_path) {
    try {
        IAMAIHandle* handle = new IAMAIHandle();
        handle->interface = new Interface(model_path);
        return handle;
    } catch (...) {
        return nullptr;
    }
}

// Free the instance
__declspec(dllexport) void iamai_free(IAMAIHandle* handle) {
    if (handle) {
        delete handle->interface;
        delete handle;
    }
}

// Main generate function
__declspec(dllexport) bool iamai_generate(IAMAIHandle* handle, const char* prompt, char* output, int output_size) {
    if (!handle || !prompt || !output || output_size <= 0) {
        return false;
    }

    try {
        std::string result = handle->interface->generate(prompt);
        strncpy(output, result.c_str(), output_size - 1);
        output[output_size - 1] = '\0';
        return true;
    } catch (...) {
        return false;
    }
}

// Configuration functions
__declspec(dllexport) void iamai_set_max_tokens(IAMAIHandle* handle, int max_tokens) {
    if (handle) {
        handle->interface->setMaxTokens(max_tokens);
    }
}

__declspec(dllexport) void iamai_set_threads(IAMAIHandle* handle, int threads) {
    if (handle) {
        handle->interface->setThreads(threads);
    }
}

__declspec(dllexport) void iamai_set_batch_size(IAMAIHandle* handle, int batch_size) {
    if (handle) {
        handle->interface->setBatchSize(batch_size);
    }
}

} // extern "C"
