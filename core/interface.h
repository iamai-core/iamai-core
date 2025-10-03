#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <stdexcept>
#include <deque>

#include "llama.h"

class Interface {
public:
    struct Config {
        int ctx = 512;
        int batch = 512;
        int max_tokens = 64;
        int threads = 4;

        // KV cache management settings
        float cache_keep_ratio = 0.75f;  // Keep 75% of context when full
        int min_keep_tokens = 512;       // Always keep at least this many tokens

        int top_k = 50;
        float top_p = 0.9f;
        float temperature = 0.7f;
        uint32_t seed = LLAMA_DEFAULT_SEED;
    };
    Config config;

    void setMaxTokens(int tokens) { config.max_tokens = tokens; }
    void setPromptFormat(const std::string& promptFormat);
    void clearPromptFormat();
    void clearContext();  // Method to clear KV cache
    int getContextUsage(); // Get current context usage
    int getContextSize();  // Get total context size

    Interface(const std::string& modelPath);
    Interface(const std::string& modelPath, Config config);
    ~Interface();

    // Main inference method - maintains context across calls
    std::string generate(const std::string& prompt);

private:
    llama_context* ctx = nullptr;
    llama_model* model = nullptr;
    const llama_vocab* vocab = nullptr;
    llama_sampler* sampler = nullptr;
    llama_memory_t memory = nullptr;

    bool formatPrompt = false;
    bool hasTemplate = false;
    std::string chatTemplate;  // Store the chat template string
    llama_token stop_token = LLAMA_TOKEN_NULL;  // Stop token for chat templates

    // KV cache state tracking
    int n_past = 0;                    // Current position in context
    std::deque<llama_token> token_history;  // Track all tokens for context management
    static const llama_seq_id MAIN_SEQ = 0; // Main sequence ID

    void loadModel(const std::string& modelPath);     // Pure model loading
    void setThreadDefaults();                         // Set default thread count
    void initializeContext();  // Context and sampler setup
    std::string applyChatTemplate(const std::string& userMessage);

    // Enhanced context management
    void manageContext(const std::vector<llama_token>& new_tokens);
    void shiftContext(int tokens_to_remove);
    bool canFitTokens(int num_tokens);

    // Helper methods
    std::string sampleTokens(bool& should_stop, bool is_first);
    std::vector<llama_token> tokenize(const std::string& text, bool add_bos = true, bool parse_special = false);
    void evaluateTokens(const std::vector<llama_token>& tokens);
};

#endif // INTERFACE_H
