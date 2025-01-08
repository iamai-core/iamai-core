#include <iostream>
#include <stdexcept>
#include <chrono>
#include "interface.h"

int main(int argc, char** argv) {
    try {
        std::cout << "Starting model initialization...\n" << std::endl;
        
        // Initialize interface with model path
        const std::string model_path = "../../../models/llama-3.2-1b-instruct-q4_k_m.gguf";
        Interface myInterface(model_path);

        // Configure generation parameters
        myInterface.setMaxTokens(256);    // Generate up to 256 tokens
        myInterface.setThreads(8);        // Reduced CPU threads since we're using GPU
        myInterface.setBatchSize(512);    // Increased batch size for GPU efficiency

        std::cout << "\nModel initialized. Starting generation test...\n" << std::endl;

        // Test prompt for generation (deliberately computation-heavy)
        const std::string prompt = "Write a detailed technical explanation of quantum computing and its applications:";
        std::cout << "Prompt: " << prompt << std::endl << std::endl;

        // Time the generation
        auto start = std::chrono::high_resolution_clock::now();
        
        // Generate text
        std::string result = myInterface.generate(prompt);
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;

        std::cout << "Generated text: " << result << std::endl;
        std::cout << "\nGeneration took " << diff.count() << " seconds" << std::endl;
        std::cout << "Tokens per second: " << 256.0 / diff.count() << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}