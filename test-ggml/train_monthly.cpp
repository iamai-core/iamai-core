#include "interface.h"
#include "m4_config.h"
#include "file_utils.h"
#include <string>

int main() {
    fprintf(stderr, "Starting M4 Monthly Series Training\n");
    fprintf(stderr, "- Using frequency: %s (Monthly)\n", m4::FREQ_MONTHLY.c_str());
    
    // Check files exist
    if (!file_utils::file_exists(m4::INFO_FILE)) {
        fprintf(stderr, "\nFile not found: %s\n", m4::INFO_FILE.c_str());
        fprintf(stderr, "Current working directory: ");
        #ifdef _WIN32
            system("cd");
        #else
            system("pwd");
        #endif
        return 1;
    }

    M4Interface interface;

    fprintf(stderr, "\nInitializing with files:\n");
    fprintf(stderr, "- Info:     %s\n", m4::INFO_FILE.c_str());
    fprintf(stderr, "- Training: %s\n", m4::TRAIN_FILE.c_str());

    if (!interface.init(m4::INFO_FILE, m4::TRAIN_FILE, m4::FREQ_MONTHLY)) {
        fprintf(stderr, "Failed to initialize\n");
        return 1;
    }

    fprintf(stderr, "Successfully loaded data files\n");

    fprintf(stderr, "\nStarting training:\n");
    fprintf(stderr, "- Model will be saved to: %s\n", m4::MODEL_FILE.c_str());
    fprintf(stderr, "- Using backend: %s\n", m4::DEFAULT_BACKEND.c_str());

    if (!interface.train(m4::MODEL_FILE, m4::DEFAULT_BACKEND)) {
        fprintf(stderr, "Training failed\n");
        return 1;
    }

    fprintf(stdout, "Training completed successfully\n");
    return 0;
}