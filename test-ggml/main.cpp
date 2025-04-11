#include "interface.h"
#include "m4_dataset.h"
#include "timeseries_common.h"
#include <cstdio>
#include <cstring>
#include <string>

int main(int argc, char ** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  Train:    %s train info.csv train.csv frequency model.gguf [CPU/CUDA0]\n", argv[0]);
        fprintf(stderr, "  Evaluate: %s eval model.gguf test.csv [CPU/CUDA0]\n", argv[0]);
        fprintf(stderr, "Frequencies: Yearly, Quarterly, Monthly, Weekly, Daily, Hourly\n");
        return 1;
    }

    M4Interface interface;
    std::string command = argv[1];

    if (command == "train") {
        if (argc < 6) {
            fprintf(stderr, "Insufficient arguments for train command\n");
            return 1;
        }

        const char* info_file = argv[2];
        const char* train_file = argv[3];
        const char* frequency = argv[4];
        const char* model_path = argv[5];
        const char* backend = argc > 6 ? argv[6] : "CPU";

        if (!interface.init(info_file, train_file, frequency)) {
            fprintf(stderr, "Failed to initialize\n");
            return 1;
        }

        if (!interface.train(model_path, backend)) {
            fprintf(stderr, "Training failed\n");
            return 1;
        }

    } else if (command == "eval") {
        if (argc < 4) {
            fprintf(stderr, "Insufficient arguments for eval command\n");
            return 1;
        }

        const char* model_path = argv[2];
        const char* test_file = argv[3];
        const char* backend = argc > 4 ? argv[4] : "CPU";

        if (!interface.evaluate(model_path, test_file, backend)) {
            fprintf(stderr, "Evaluation failed\n");
            return 1;
        }

    } else {
        fprintf(stderr, "Unknown command: %s\n", command.c_str());
        return 1;
    }

    return 0;
}