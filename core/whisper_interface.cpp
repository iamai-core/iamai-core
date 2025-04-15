#include "whisper_interface.h"
#include <stdexcept>
#include <vector>
#include <fstream>
#include <cstring>
#include <iostream>

WhisperInterface::WhisperInterface(const std::string& model_path, int threads) {

    n_threads = threads;
    ctx = whisper_init_from_file(model_path.c_str());
    if (!ctx) throw std::runtime_error("Failed to initialize whisper context");

}

WhisperInterface::~WhisperInterface() {
    if (ctx) {
        whisper_free(ctx);
        ctx = nullptr;
    }
}

void WhisperInterface::setThreads(int n_threads) {
    this->n_threads = n_threads;
}

void WhisperInterface::setLanguage(const std::string& language) {
    this->language = language;
}

void WhisperInterface::setTranslate(bool translate) {
    this->translate = translate;
}

void WhisperInterface::validateContext() const {
    if (!ctx) {
        throw std::runtime_error("Whisper context not initialized");
    }
}

std::string WhisperInterface::transcribe(const std::string& audio_path) {
    validateContext();
    std::vector<float> pcmf32;
    if (!loadAudioFile(audio_path, pcmf32)) {
        throw std::runtime_error("Failed to load audio file");
    }
    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_progress = true;
    params.print_special = false;
    params.print_realtime = false;
    params.print_timestamps = false;
    params.translate = translate;
    params.language = language.c_str();
    params.n_threads = n_threads;

    if (whisper_full(ctx, params, pcmf32.data(), pcmf32.size()) != 0) {
        throw std::runtime_error("Failed to run whisper");
    }
    std::string result;
    const int n_segments = whisper_full_n_segments(ctx);
    for (int i = 0; i < n_segments; ++i) {
        const char* text = whisper_full_get_segment_text(ctx, i);
        result += text;
        result += " ";
    }

    return result;

}

std::string WhisperInterface::transcribe( float* data, int samples ) {

    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_progress = true;
    params.print_special = false;
    params.print_realtime = false;
    params.print_timestamps = false;
    params.translate = translate;
    params.language = language.c_str();
    params.n_threads = n_threads;

    if (whisper_full(ctx, params, data, samples) != 0) {
        throw std::runtime_error("Failed to run whisper");
    }
    std::string result;
    const int n_segments = whisper_full_n_segments(ctx);
    for (int i = 0; i < n_segments; ++i) {
        const char* text = whisper_full_get_segment_text(ctx, i);
        result += text;
        result += " ";
    }

    std::cout << "Test print" << result << std::endl; 

    return result.c_str();
    
}

bool WhisperInterface::loadAudioFile(const std::string& audio_path, std::vector<float>& pcmf32) {
    std::cout << "Opening audio file: " << audio_path << std::endl;
    std::ifstream file(audio_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open audio file" << std::endl;
        return false;
    }
    char riff_header[12];
    file.read(riff_header, 12);
    if (!file || memcmp(riff_header, "RIFF", 4) != 0 || memcmp(riff_header+8, "WAVE", 4) != 0) {
        std::cerr << "Invalid WAV file (RIFF header)" << std::endl;
        return false;
    }
    uint32_t chunk_size;
    char chunk_header[8];
    bool fmt_found = false, data_found = false;
    uint16_t audio_format, num_channels, block_align, bits_per_sample;
    uint32_t sample_rate, byte_rate;

    while (!data_found && file.read(chunk_header, 8)) {
        memcpy(&chunk_size, chunk_header + 4, 4);

        if (memcmp(chunk_header, "fmt ", 4) == 0) {
            char fmt_data[16];
            file.read(fmt_data, 16);
            memcpy(&audio_format, fmt_data, 2);
            memcpy(&num_channels, fmt_data + 2, 2);
            memcpy(&sample_rate, fmt_data + 4, 4);
            memcpy(&byte_rate, fmt_data + 8, 4);
            memcpy(&block_align, fmt_data + 12, 2);
            memcpy(&bits_per_sample, fmt_data + 14, 2);

            if (audio_format != 1) {
                std::cerr << "Non-PCM format not supported" << std::endl;
                return false;
            }
            fmt_found = true;
        }
        else if (memcmp(chunk_header, "data", 4) == 0) {
            data_found = true;
            break;
        }
        else {
            file.ignore(chunk_size);
        }
    }

    if (!fmt_found || !data_found) {
        std::cerr << "Invalid WAV file (missing fmt/data chunks)" << std::endl;
        return false;
    }
    if (bits_per_sample != 16) {
        std::cerr << "Only 16-bit PCM supported" << std::endl;
        return false;
    }
    std::vector<int16_t> pcm16(chunk_size / 2);
    file.read(reinterpret_cast<char*>(pcm16.data()), chunk_size);
    if (!file) {
        std::cerr << "Failed to read PCM data" << std::endl;
        return false;
    }
    pcmf32.resize(pcm16.size());
    for (size_t i = 0; i < pcm16.size(); i++) {
        pcmf32[i] = static_cast<float>(pcm16[i]) / 32768.0f;
    }

    std::cout << "Audio file loaded successfully. Samples: " << pcmf32.size()
              << ", Duration: " << (pcmf32.size()/16000.0f) << "s" << std::endl;
    return true;
}