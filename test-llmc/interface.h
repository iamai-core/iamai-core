#ifndef LLMC_INTERFACE_H
#define LLMC_INTERFACE_H

#include <string>
#include <vector>
#include <memory>

// Forward declarations
struct GPT2;
struct Tokenizer;
struct DataLoader;

class LLMCInterface {
public:
    LLMCInterface(const std::string& modelPath);
    ~LLMCInterface();

    // Configure generation parameters
    void setMaxTokens(int n) { max_tokens = n; }
    void setTemperature(float t) { temperature = t; }
    void setTopK(int k) { top_k = k; }

    // Main inference method
    std::string generate(const std::string& prompt);

private:
    // LLMC model state
    GPT2* model;
    Tokenizer* tokenizer;
    DataLoader* loader;

    // Generation parameters
    int max_tokens = 64;     // Max tokens to generate
    float temperature = 1.0f; // Sampling temperature
    int top_k = 40;          // Top-k sampling parameter
    int batch_size = 1;      // Batch size for inference
    int seq_len = 1024;      // Sequence length

    // Helper methods
    std::string sampleTokens(unsigned long long& rng_state);
    void initializeModel(const std::string& modelPath);
};

#endif // LLMC_INTERFACE_H
