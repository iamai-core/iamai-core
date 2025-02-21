#include "interface.h"
#include "m4_config.h"
#include <string>
#include "file_utils.h"

int main() {
    M4Interface interface;

    if (!interface.init(m4::INFO_FILE, m4::TRAIN_FILE, m4::DEFAULT_FREQUENCY)) {
        file_utils::check_file_exists(m4::INFO_FILE);
        file_utils::check_file_exists(m4::TRAIN_FILE);
        fprintf(stderr, "Failed to initialize with files:\n");
        fprintf(stderr, "Info: %s\n", m4::INFO_FILE.c_str());
        fprintf(stderr, "Train: %s\n", m4::TRAIN_FILE.c_str());
        return 1;
    }

    if (!interface.evaluate(m4::MODEL_FILE, m4::TRAIN_FILE, m4::DEFAULT_BACKEND)) {
        fprintf(stderr, "Evaluation failed\n");
        return 1;
    }

    fprintf(stdout, "Evaluation completed successfully\n");
    return 0;
}