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
        std::string model_path = (exe_dir / "llama-3.2-1b-instruct-q4_k_m.gguf").string();

        // Initialize interface with model path
        Interface myInterface(model_path);

        std::cout << "Model initialized. Ready for input." << std::endl;

        // Get prompt from user
        std::cout << "Enter your prompt (press Enter twice when done):\n";
        std::string prompt = "What colors is a rainbow";
        std::string line;
        /*
        // Read lines until an empty line is entered
        while (std::getline(std::cin, line) && !line.empty()) {
            prompt += line + "\n";
        }

        // Remove the last newline if it exists
        if (!prompt.empty() && prompt.back() == '\n') {
            prompt.pop_back();
        }
*/
        std::cout << "Generating response for prompt: " << prompt << std::endl;

        // Time the generation
        auto start = std::chrono::high_resolution_clock::now();

        // Generate text
        std::string result = myInterface.generate(prompt);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;

        std::cout << "Generated text: " << result << std::endl << std::endl;
        std::cout << "Generation took " << diff.count() << " seconds" << std::endl;
        std::cout << "Tokens per second: " << 256.0 / diff.count() << std::endl;

        return 0;
    }
    
    catch (const std::exception& e) {

        std::cerr << "Error: " << e.what() << std::endl;
        return 1;

    }

}
