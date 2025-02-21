#pragma once

#include <ggml.h>
#include <ggml-opt.h>
#include "timeseries_common.h"  // For TS_* constants
#include <string>
#include <vector>

struct M4Series {
    std::string id;
    std::string type;  // "Yearly", "Quarterly", "Monthly", "Weekly", "Daily", "Hourly"
    std::vector<float> values;
};

class M4Dataset {
public:
    std::vector<M4Series> series;
    
    // Load series information from M4-info.csv
    bool load_info(const std::string& info_file);
    
    // Load actual time series data from train/test files
    bool load_data(const std::string& data_file);
    
    // Normalize all series (zero mean, unit variance)
    void normalize_series();
    
    // Prepare data for training/testing
    bool prepare_training_data(ggml_opt_dataset_t dataset, const std::string& freq_filter = "");
    
    // Get statistics about the dataset
    size_t get_series_count(const std::string& freq_filter = "") const;
    size_t get_total_observations(const std::string& freq_filter = "") const;
};