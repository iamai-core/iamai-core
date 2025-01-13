#include <iostream>
#include <chrono>
#include <string>
#include "interface.h"

int main(int argc, char** argv) {
    try {
        std::cout << "Starting LLMC model initialization...\n" << std::endl;

        // Initialize interface with model path - change path as needed
        const std::string model_path = "gpt2_124M_bf16.bin";
        LLMCInterface llmc(model_path);

        // Configure generation parameters
        llmc.setMaxTokens(64);
        llmc.setTemperature(0.8f);
        llmc.setTopK(40);

        std::cout << "\nModel initialized. Ready for input.\n" << std::endl;

        // Get prompt from user
        std::cout << "Enter your prompt (press Enter twice when done):\n";
        std::string prompt;
        std::string line;

        while (std::getline(std::cin, line) && !line.empty()) {
            prompt += line + "\n";
        }

        // Remove trailing newline
        if (!prompt.empty() && prompt.back() == '\n') {
            prompt.pop_back();
        }

        std::cout << "\nGenerating response for prompt: " << prompt << "\n" << std::endl;

        // Time the generation
        auto start = std::chrono::high_resolution_clock::now();

        // Generate text
        std::string result = llmc.generate(prompt);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;

        // Output results
        std::cout << "Generated text: " << result << std::endl;
        std::cout << "\nGeneration took " << diff.count() << " seconds" << std::endl;
        std::cout << "Tokens per second: " << 64.0 / diff.count() << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
