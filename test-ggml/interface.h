#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <stdexcept>
#include "llama.h"

class Interface {
public:
    Interface(const std::string& modelPath);
    ~Interface();

    // Configure generation parameters
    void setMaxTokens(int n) { max_tokens = n; }
    void setThreads(int n) { n_threads = n; }
    void setBatchSize(int n) { n_batch = n; }

    // Main inference method
    std::string generate(const std::string& prompt);

private:
    llama_context* ctx;
    llama_model* model;
    llama_sampler* sampler;

    int max_tokens = 32;    // Default max tokens to generate
    int n_threads = 4;      // Default number of threads
    int n_batch = 8;        // Default batch size
    int n_ctx = 512;        // Context size

    // Helper method for token sampling
    std::string sampleTokens(int& n_past, bool& should_stop);
};

#endif // INTERFACE_H
