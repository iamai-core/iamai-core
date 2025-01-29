#include <iostream>
#include <chrono>
#include "whisper_interface.h"

int main(int argc, char** argv) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <whisper_model_path> <audio_file_path>" << std::endl;
            return 1;
        }

        std::string whisper_model_path = argv[1];
        std::string audio_path = argv[2];

        // Initialize Whisper
        std::cout << "Initializing Whisper model..." << std::endl;
        WhisperInterface whisper(whisper_model_path);
        whisper.setThreads(8);  // Adjust based on your system
        
        // Transcribe audio
        std::cout << "Transcribing audio..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        std::string transcription = whisper.transcribe(audio_path);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> diff = end - start;
        std::cout << "Transcription took " << diff.count() << " seconds" << std::endl;
        std::cout << "Transcribed text: " << transcription << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}