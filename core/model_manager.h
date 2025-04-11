#pragma once
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <vector>
#include <memory>
#include "folder_manager.h"
#include "interface.h"

namespace iamai {

class ModelManager {
private:
    std::filesystem::path models_dir;
    std::unique_ptr<Interface> current_model;

public:
    ModelManager() {
        auto& folder_manager = FolderManager::getInstance();
        models_dir = folder_manager.getModelsPath();
        
        if (!std::filesystem::exists(models_dir)) {
            throw std::runtime_error("Models directory does not exist: " + models_dir.string());
        }
    }

    std::vector<std::string> listModels() {
        std::vector<std::string> models;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(models_dir)) {
                if (entry.path().extension() == ".gguf") {
                    models.push_back(entry.path().filename().string());
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error listing models: " << e.what() << std::endl;
        }
        return models;
    }

    bool switchModel(const std::string& model_name) {
        try {
            std::filesystem::path model_path = models_dir / model_name;
            if (!std::filesystem::exists(model_path)) {
                std::cerr << "Model file not found: " << model_path.string() << std::endl;
                return false;
            }

            auto new_model = std::make_unique<Interface>(model_path.string());
            // Configure the model parameters
            new_model->setMaxTokens(512);
            new_model->setThreads(1);
            new_model->setBatchSize(8);

            current_model = std::move(new_model);
            std::cout << "Successfully switched to model: " << model_name << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error switching model: " << e.what() << std::endl;
            return false;
        }
    }

    Interface* getCurrentModel() {
        return current_model.get();
    }
};

} // namespace iamai