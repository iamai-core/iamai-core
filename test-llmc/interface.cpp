#include "interface.h"
#include <stdexcept>
#include <iostream>
#include <cstring>

// Include llmc headers
extern "C" {
#include "train_gpt2.h"
#include "tokenizer.h"
#include "dataloader.h"
#include "sampler.h"
}

LLMCInterface::LLMCInterface(const std::string& modelPath) {
    try {
        initializeModel(modelPath);
    } catch (const std::exception& e) {
        std::cerr << "Error initializing model: " << e.what() << std::endl;
        throw;
    }
}

LLMCInterface::~LLMCInterface() {
    if (tokenizer) {
        tokenizer_free(tokenizer);
        delete tokenizer;
    }
    if (model) {
        gpt2_free(model);
        delete model;
    }
    if (loader) {
        dataloader_free(loader);
        delete loader;
    }
}

void LLMCInterface::initializeModel(const std::string& modelPath) {
    // Initialize model
    model = new GPT2();
    gpt2_build_from_checkpoint(model, modelPath.c_str());

    // Initialize tokenizer
    tokenizer = new Tokenizer();
    tokenizer_init(tokenizer, "gpt2_tokenizer.bin");

    // Initialize dataloader (for inference we use small batch size)
    loader = new DataLoader();
    dataloader_init(loader, "", batch_size, seq_len, 0, 1, 0);
}

std::string LLMCInterface::sampleTokens(unsigned long long& rng_state) {
    std::string result;

    // Get logits from last prediction
    float* logits = model->acts.logits;
    float* probs = model->acts.probs;

    // Sample next token using temperature and top-k
    float coin = random_f32(&rng_state);
    int next_token = sample_softmax(logits, model->config.vocab_size, coin);

    // Convert token to string
    if (tokenizer->init_ok) {
        const char* token_str = tokenizer_decode(tokenizer, next_token);
        if (token_str) {
            result = token_str;
        }
    }

    return result;
}

std::string LLMCInterface::generate(const std::string& prompt) {
    std::string result;
    unsigned long long rng_state = 1337; // For reproducibility

    // Prepare input tokens array
    std::vector<int> tokens(seq_len, tokenizer->eot_token);

    // Forward pass on the model
    gpt2_forward(model, tokens.data(), nullptr, batch_size, seq_len);

    // Generate tokens
    for (int i = 0; i < max_tokens; i++) {
        result += sampleTokens(rng_state);
    }

    return result;
}
