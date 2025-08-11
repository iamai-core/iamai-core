#pragma once
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <vector>
#include <memory>
#include "../core/interface.h"
#include "../core/folder_manager.h"

namespace iamai {

class ModelManager {
private:
    std::filesystem::path models_dir;
    std::unique_ptr<Interface> current_model;

public:
    ModelManager();

    std::vector<std::string> listModels();
    bool switchModel(const std::string& model_name);
    Interface* getCurrentModel();
};

} // namespace iamai
