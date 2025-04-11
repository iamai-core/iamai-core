#pragma once

#include <string>

namespace m4 {
    // Build configuration will be three directories up from bin-ggml/Debug
    const std::string PROJECT_ROOT = "../../../";
    
    // Data paths relative to project root
    const std::string DATA_DIR = PROJECT_ROOT + "data";
    const std::string INFO_FILE = DATA_DIR + "/M4-info.csv";
    const std::string TRAIN_FILE = DATA_DIR + "/Monthly-train.csv";
    const std::string TEST_FILE = DATA_DIR + "/Monthly-test.csv";
    
    // Model paths (relative to where executable runs)
    const std::string MODEL_DIR = ".";
    const std::string MODEL_FILE = MODEL_DIR + "/model.gguf";
    
    // Default settings
    const std::string DEFAULT_BACKEND = "CPU";
    
    // M4 frequency types - using numeric values from the dataset
    const std::string FREQ_YEARLY = "1";
    const std::string FREQ_QUARTERLY = "4";
    const std::string FREQ_MONTHLY = "12";
    const std::string FREQ_BIMONTHLY = "24";
    
    const std::string DEFAULT_FREQUENCY = FREQ_MONTHLY;  // 12 = Monthly
}