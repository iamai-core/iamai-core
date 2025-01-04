#include <iostream>
#include <stdexcept>
#include "interface.h"

int main() {
    try {
        // Initialize interface with hardcoded model path
        // TODO: Replace with your actual model path
        const std::string model_path = "models/tinyllama-1b-1431k-3T-q8_0.gguf";
        Interface myInterface(model_path);

        // Test tokenization
        const std::string test_text = "Hello, this is a test!";
        std::cout << "Input text: " << test_text << std::endl;

        myInterface.share(test_text);
        std::vector<int> tokens = myInterface.collect();

        // Print tokens
        std::cout << "Tokens (" << tokens.size() << "): ";
        for (size_t i = 0; i < tokens.size(); i++) {
            if (i > 0) {
                std::cout << ", ";
            }
            std::cout << tokens[i];
        }
        std::cout << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
