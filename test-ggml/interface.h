#pragma once

#include <ggml.h>
#include "m4_dataset.h"
#include "timeseries_common.h"
#include <string>

class M4Interface {
public:
    M4Interface();
    ~M4Interface();

    // Initialize the M4 dataset
    bool init(const std::string& info_file, const std::string& data_file, const std::string& frequency);

    // Train a model
    bool train(const std::string& model_path, const std::string& backend = "CPU");

    // Evaluate a model
    bool evaluate(const std::string& model_path, const std::string& test_data_file, const std::string& backend = "CPU");

private:
    M4Dataset m4_data;
    TimeseriesModel* model;
    std::string current_frequency;
    bool is_initialized;
};