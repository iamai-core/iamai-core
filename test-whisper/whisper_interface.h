// whisper_interface.h
#pragma once
#include <string>
#include <vector>
#include "whisper.h"

class WhisperInterface {
public:
    WhisperInterface(const std::string& model_path);
    ~WhisperInterface();

    // Configure parameters
    void setThreads(int n_threads);
    void setLanguage(const std::string& language);
    void setTranslate(bool translate);

    // Main functionality
    std::string transcribe(const std::string& audio_path);

private:
    whisper_context* ctx;
    int n_threads = 4;
    std::string language = "en";
    bool translate = false;
    
    void validateContext() const;
    bool loadAudioFile(const std::string& audio_path, std::vector<float>& pcmf32);
};