#include <iostream>
#include <stdexcept>
#include "interface.h"

int main(int argc, char** argv) {
    try {
        // Initialize interface with model path
        // TODO: Replace with your actual model path
        const std::string model_path = "../../../models/tinyllama-2-1b-miniguanaco.Q4_K_M.gguf";
        Interface myInterface(model_path);

        // Configure generation parameters
        myInterface.setMaxTokens(256);     // Generate up to 256 tokens
        myInterface.setThreads(6);        // Use 6 threads
        myInterface.setBatchSize(1);      // Process 1 token at a time

        // Test prompt for generation
        const std::string prompt = "Write a short story about a robot learning to paint:";
        std::cout << "Prompt: " << prompt << std::endl << std::endl;

        // Generate text
        std::string result = myInterface.generate(prompt);
        std::cout << "Generated text: " << result << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
