#include <iostream>
#include <chrono>
#include <string>

#ifdef _WIN32
#include <windows.h>
#define LOAD_LIBRARY(name) LoadLibraryA(name)
#define GET_PROC_ADDRESS(handle, name) GetProcAddress(handle, name)
#define FREE_LIBRARY(handle) FreeLibrary(handle)
typedef HMODULE LibraryHandle;
#else
#include <dlfcn.h>
#define LOAD_LIBRARY(name) dlopen(name, RTLD_LAZY)
#define GET_PROC_ADDRESS(handle, name) dlsym(handle, name)
#define FREE_LIBRARY(handle) dlclose(handle)
typedef void* LibraryHandle;
#endif

// Function pointer types matching interface-lib.cpp
typedef void* (*InitFunc)(const char*);
typedef bool (*GenerateFunc)(void*, const char*, char*, int);
typedef void (*SetMaxTokensFunc)(void*, int);
typedef void (*FreeFunc)(void*);

int main(int argc, char** argv) {
    LibraryHandle library = nullptr;
    void* context = nullptr;

    std::cout << "Loading iamai-core library...\n" << std::endl;

    // Load the library
#ifdef _WIN32
    library = LOAD_LIBRARY("iamai-core.dll");
#elif defined(__APPLE__)
    library = LOAD_LIBRARY("./libiamai-core.dylib");
#else
    library = LOAD_LIBRARY("./libiamai-core.so");
#endif

    if (!library) {
        std::cerr << "Error: Failed to load iamai-core library" << std::endl;
        return 1;
    }

    // Get function pointers
    InitFunc Init = (InitFunc)GET_PROC_ADDRESS(library, "Init");
    GenerateFunc Generate = (GenerateFunc)GET_PROC_ADDRESS(library, "Generate");
    SetMaxTokensFunc SetMaxTokens = (SetMaxTokensFunc)GET_PROC_ADDRESS(library, "SetMaxTokens");
    FreeFunc Free = (FreeFunc)GET_PROC_ADDRESS(library, "Free");

    if (!Init || !Generate || !SetMaxTokens || !Free) {
        std::cerr << "Error: Failed to get function pointers from library" << std::endl;
        FREE_LIBRARY(library);
        return 1;
    }

    std::cout << "Initializing model...\n" << std::endl;

    // Initialize interface with model path
    const std::string model_path = "./models/Llama-3.2-1B-Instruct-Q4_K_M.gguf";
    context = Init(model_path.c_str());

    if (!context) {
        std::cerr << "Error: Failed to initialize model" << std::endl;
        FREE_LIBRARY(library);
        return 1;
    }

    std::cout << "Model initialized. Ready for input." << std::endl;

    std::string prompt = "What colors is a rainbow";

    std::cout << "Generating response for prompt: " << prompt << std::endl;

    // Time the generation
    auto start = std::chrono::high_resolution_clock::now();

    // Generate text
    const int output_size = 4096;
    char output[output_size];
    bool success = Generate(context, prompt.c_str(), output, output_size);

    if (!success) {
        std::cerr << "Error: Text generation failed" << std::endl;
        Free(context);
        FREE_LIBRARY(library);
        return 1;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Generated text: " << output << std::endl << std::endl;
    std::cout << "Generation took " << diff.count() << " seconds" << std::endl;
    std::cout << "Tokens per second: " << 256.0 / diff.count() << std::endl;

    // Cleanup
    Free(context);
    FREE_LIBRARY(library);

    return 0;
}
