#include <iostream>
#include <chrono>
#include <filesystem>
#include "whisper_interface.h"

int main() {
    try {
        // Hardcoded paths
        const std::string whisper_model_path = "..\\..\\..\\whisper.cpp\\models\\ggml-base.en.bin";
        const std::string audio_path = "..\\..\\..\\whisper.cpp\\samples\\jfk.wav";

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