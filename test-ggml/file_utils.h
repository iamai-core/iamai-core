#pragma once

#include <string>
#include <fstream>

namespace file_utils {
    inline bool file_exists(const std::string& path) {
        std::ifstream f(path.c_str());
        return f.good();
    }

    inline void check_file_exists(const std::string& path) {
        if (!file_exists(path)) {
            fprintf(stderr, "File not found: %s\n", path.c_str());
            fprintf(stderr, "Current working directory might be incorrect\n");
            exit(1);
        }
    }
}