// whisper_interface.cpp
#include "whisper_interface.h"
#include <stdexcept>
#include <vector>
#include <fstream>
#include <cstring>
#include <iostream>

WhisperInterface::WhisperInterface(const std::string& model_path) {
    std::cout << "Loading model from: " << model_path << std::endl;
    ctx = whisper_init_from_file(model_path.c_str());
    if (!ctx) {
        throw std::runtime_error("Failed to initialize whisper context");
    }
    std::cout << "Model loaded successfully" << std::endl;
}

WhisperInterface::~WhisperInterface() {
    if (ctx) {
        whisper_free(ctx);
    }
}

void WhisperInterface::setThreads(int threads) {
    n_threads = threads;
}

void WhisperInterface::setLanguage(const std::string& lang) {
    language = lang;
}

void WhisperInterface::setTranslate(bool should_translate) {
    translate = should_translate;
}

void WhisperInterface::validateContext() const {
    if (!ctx) {
        throw std::runtime_error("Whisper context is not initialized");
    }
}

bool WhisperInterface::loadAudioFile(const std::string& audio_path, std::vector<float>& pcmf32) {
    std::cout << "Opening audio file: " << audio_path << std::endl;
    std::ifstream file(audio_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open audio file" << std::endl;
        return false;
    }

    // Read WAV header
    char header[44];
    file.read(header, 44);
    if (!file) {
        std::cerr << "Failed to read WAV header" << std::endl;
        return false;
    }

    // Verify WAV format
    if (header[0] != 'R' || header[1] != 'I' || header[2] != 'F' || header[3] != 'F') {
        std::cerr << "Invalid WAV format (RIFF header not found)" << std::endl;
        return false;
    }

    // Get data size from header
    uint32_t data_size;
    std::memcpy(&data_size, &header[40], 4);
    std::cout << "Audio data size: " << data_size << " bytes" << std::endl;

    // Read PCM data
    std::vector<int16_t> pcm16;
    pcm16.resize(data_size / 2);
    file.read(reinterpret_cast<char*>(pcm16.data()), data_size);
    if (!file) {
        std::cerr << "Failed to read PCM data" << std::endl;
        return false;
    }

    // Convert to float32
    pcmf32.resize(pcm16.size());
    for (size_t i = 0; i < pcm16.size(); i++) {
        pcmf32[i] = static_cast<float>(pcm16[i]) / 32768.0f;
    }

    std::cout << "Audio file loaded successfully. Samples: " << pcmf32.size() << std::endl;
    return true;
}

std::string WhisperInterface::transcribe(const std::string& audio_path) {
    std::cout << "Starting transcription process..." << std::endl;
    validateContext();

    // Load audio file
    std::vector<float> pcmf32;
    if (!loadAudioFile(audio_path, pcmf32)) {
        throw std::runtime_error("Failed to load audio file");
    }

    // Initialize parameters
    std::cout << "Initializing Whisper parameters..." << std::endl;
    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    
    params.print_progress   = true;  // Enable progress printing
    params.print_special    = false;
    params.print_realtime   = false;
    params.print_timestamps = false;
    params.translate       = translate;
    params.language        = language.c_str();
    params.n_threads       = n_threads;

    std::cout << "Processing audio with parameters:" << std::endl
              << "- Language: " << language << std::endl
              << "- Threads: " << n_threads << std::endl
              << "- Translate: " << (translate ? "yes" : "no") << std::endl;

    // Process the audio
    std::cout << "Running Whisper inference..." << std::endl;
    if (whisper_full(ctx, params, pcmf32.data(), pcmf32.size()) != 0) {
        throw std::runtime_error("Failed to process audio");
    }

    // Extract the transcribed text
    std::cout << "Extracting transcribed text..." << std::endl;
    std::string result;
    const int n_segments = whisper_full_n_segments(ctx);
    std::cout << "Found " << n_segments << " segments" << std::endl;
    
    for (int i = 0; i < n_segments; ++i) {
        const char* text = whisper_full_get_segment_text(ctx, i);
        result += text;
        if (i < n_segments - 1) {
            result += " ";
        }
    }

    std::cout << "Transcription complete" << std::endl;
    return result;
}