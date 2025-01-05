#include <iostream>
#include <stdexcept>
#include "interface.h"

int main(int argc, char** argv) {
    try {
        // Initialize interface with hardcoded model path
        // TODO: Replace with your actual model path
        const std::string model_path = "models/tinyllama-1b-1431k-3T-q8_0.gguf";
        Interface myInterface(model_path);

        // Test tokenization and detokenization
        const std::string test_text = "Hello, this is a test!";
        std::cout << "Original text: " << test_text << std::endl;

        // Convert to tokens
        myInterface.share(test_text);

        // Convert back to text
        std::string result = myInterface.collect();
        std::cout << "Reconstructed text: " << result << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
