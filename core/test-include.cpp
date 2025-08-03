#include <iostream>
#include <stdexcept>
#include <chrono>
#include <string>
#include <limits>
#include "interface.h"

int main(int argc, char** argv) {
    try {
        std::cout << "Starting model initialization...\n" << std::endl;

        // Initialize interface with model path
        const std::string model_path = "models/Llama-3.2-1B-Instruct-Q4_K_M.gguf";
        Interface myInterface(model_path);

        std::cout << "Model initialized. Ready for input." << std::endl;

        std::string prompt = "What colors is a rainbow";

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
