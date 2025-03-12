#include <iostream>
#include <stdexcept>
#include <chrono>
#include <string>
#include <limits>
#include <filesystem>
#include "interface.h"

int main(int argc, char** argv) {
    try {
        std::cout << "Starting model initialization...\n" << std::endl;
        
        // Get current executable directory and build model path
        std::filesystem::path exe_dir = std::filesystem::current_path();
        std::string model_path = (exe_dir / "tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf").string();
        
        // Initialize interface with model path
        Interface myInterface(model_path);
        
        // Configure generation parameters
        myInterface.setMaxTokens(256);    // Generate up to 256 tokens
        myInterface.setThreads(1);        // Reduced CPU threads since we're using GPU
        myInterface.setBatchSize(1);      // Increased batch size for GPU efficiency
        
        std::cout << "\nModel initialized. Ready for input.\n" << std::endl;
        
        // Get prompt from user
        std::cout << "Enter your prompt (press Enter twice when done):\n";
        std::string prompt;
        std::string line;
        
        // Read lines until an empty line is entered
        while (std::getline(std::cin, line) && !line.empty()) {
            prompt += line + "\n";
        }
        
        // Remove the last newline if it exists
        if (!prompt.empty() && prompt.back() == '\n') {
            prompt.pop_back();
        }
        
        std::cout << "\nGenerating response for prompt: " << prompt << "\n" << std::endl;
        
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
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}