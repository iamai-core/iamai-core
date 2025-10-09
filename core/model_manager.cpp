#include "../core/interface.h"
#include "../core/folder_manager.h"
#include "../core/model_manager.h"
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <thread>

namespace iamai {

ModelManager::ModelManager() {
    auto& folder_manager = FolderManager::getInstance();
    models_dir = folder_manager.getModelsPath();

    if (!std::filesystem::exists(models_dir)) {
        throw std::runtime_error("Models directory does not exist: " + models_dir.string());
    }
}

std::vector<std::string> ModelManager::listModels() {
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

bool ModelManager::switchModel(const std::string& model_name) {
    try {
        std::filesystem::path model_path = models_dir / model_name;
        if (!std::filesystem::exists(model_path)) {
            std::cerr << "Model file not found: " << model_path.string() << std::endl;
            return false;
        }

        std::cout << "....................................................................................." << std::endl;
        if (current_model) current_model.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto new_model = std::make_unique<Interface>(model_path.string());

        current_model = std::move(new_model);
        std::cout << "Successfully switched to model: " << model_name << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error switching model: " << e.what() << std::endl;
        return false;
    }
}

Interface* ModelManager::getCurrentModel() {
    return current_model.get();
}

} // namespace iamai
