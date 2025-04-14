#include "whisper_interface.h"
#include <cstring>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

struct Context {
    WhisperInterface* interface;
};

extern "C" {


EXPORT Context* Init(const char* model_path) {
    
    try {

        Context* ctx = new Context;
        ctx->interface = new WhisperInterface(model_path);
        return ctx;

    } catch (...) {

        return nullptr;

    }

}

EXPORT void Free(Context* ctx) {
    
    if (ctx) {
        delete ctx->interface;
        delete ctx;
    }

}

EXPORT void setLanguage( Context* ctx, const char* language) {

    if (ctx) ctx->interface->setLanguage( language );

}

EXPORT void setTranslate(Context* ctx, bool translate) {

    if (ctx) ctx->interface->setTranslate( translate );

}

EXPORT const char* Transcrible( Context* ctx, float* data, int samples ) {

    try {
        
        static std::string result;
        result = ctx->interface->transcribe( data, samples );
        return result.c_str();

    } catch (...) {

        return nullptr;

    }

}

}