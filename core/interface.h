#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <stdexcept>

#include "llama.h"

class Interface {
public:
    struct Config {
        int ctx = 2048;
        int batch = 64;
        int max_tokens = 256;
        int threads = 8;

        int top_k = 50;
        float top_p = 0.9f;
        float temperature = 0.5f;
        uint32_t seed = LLAMA_DEFAULT_SEED;
    };
    Config config;

    void setMaxTokens(int tokens) { config.max_tokens = tokens; }
    void setPromptFormat(const std::string& promptFormat);
    void clearPromptFormat();

    Interface(const std::string& modelPath);
    Interface(const std::string& modelPath, Config config);
    ~Interface();

    // Main inference method
    std::string generate(const std::string& prompt);

private:
    llama_context* ctx = nullptr;
    llama_model* model = nullptr;
    const llama_vocab* vocab = nullptr;
    llama_sampler* sampler = nullptr;

    bool formatPrompt = false;
    std::string promptFormat = "";

    void initializeModel(const std::string& modelPath);
    void formatNewPrompt(const std::string& input, std::string& output);

    // Helper method for token sampling
    std::string sampleTokens(int& n_past, bool& should_stop);
};

#endif // INTERFACE_H
