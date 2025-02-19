#include "m4_dataset.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <set>
#include <map>

static inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

bool M4Dataset::load_info(const std::string& info_file) {
    std::ifstream fin(info_file);
    if (!fin) {
        fprintf(stderr, "Failed to open info file: %s\n", info_file.c_str());
        return false;
    }

    fprintf(stderr, "\nReading M4 info file...\n");
    std::string line;
    
    // Parse header to find column indices
    std::getline(fin, line);
    fprintf(stderr, "Header: '%s'\n", line.c_str());
    
    std::stringstream header_ss(line);
    std::string col;
    int id_col = -1, freq_col = -1, cat_col = -1;
    int curr_col = 0;
    
    while (std::getline(header_ss, col, ',')) {
        col = trim(col);
        if (col == "M4id") id_col = curr_col;
        else if (col == "Frequency") freq_col = curr_col;
        else if (col == "category") cat_col = curr_col;
        curr_col++;
    }
    
    if (id_col == -1 || freq_col == -1 || cat_col == -1) {
        fprintf(stderr, "Error: Required columns not found in header\n");
        return false;
    }
    
    // Track frequencies and categories
    std::set<std::string> frequencies;
    std::set<std::string> categories;
    size_t count = 0;
    
    while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string field;
        std::string id, freq, category;
        
        // Read all columns and extract the ones we need
        for (int i = 0; std::getline(ss, field, ','); i++) {
            field = trim(field);
            if (i == id_col) id = field;
            else if (i == freq_col) freq = field;
            else if (i == cat_col) category = field;
        }
        
        if (!id.empty() && !freq.empty() && !category.empty()) {
            frequencies.insert(freq);
            categories.insert(category);
            
            M4Series series;
            series.id = id;
            series.type = freq;  // Use frequency as the type
            this->series.push_back(series);
            
            if (count < 5) {  // Print first few entries for debugging
                fprintf(stderr, "Sample entry: ID='%s', Frequency='%s', Category='%s'\n", 
                    id.c_str(), freq.c_str(), category.c_str());
            }
            count++;
        } else {
            fprintf(stderr, "Warning: Malformed line: '%s'\n", line.c_str());
        }
    }
    
    fprintf(stderr, "\nInfo file summary:\n");
    fprintf(stderr, "- Total series: %zu\n", count);
    fprintf(stderr, "- Found frequencies:\n");
    for (const auto& freq : frequencies) {
        size_t freq_count = std::count_if(series.begin(), series.end(),
            [&freq](const M4Series& s) { return s.type == freq; });
        fprintf(stderr, "  * %s: %zu series\n", freq.c_str(), freq_count);
    }
    
    fprintf(stderr, "- Categories:\n");
    for (const auto& cat : categories) {
        fprintf(stderr, "  * %s\n", cat.c_str());
    }
    
    return count > 0;
}

bool M4Dataset::load_data(const std::string& data_file) {
    std::ifstream fin(data_file);
    if (!fin) {
        fprintf(stderr, "Failed to open data file: %s\n", data_file.c_str());
        return false;
    }

    fprintf(stderr, "\nReading M4 data file...\n");
    std::string line;
    
    // Examine header
    std::getline(fin, line);
    
    
    size_t idx = 0;
    size_t total_values = 0;
    size_t parse_errors = 0;
    
    while (std::getline(fin, line) && idx < series.size()) {
        std::stringstream ss(line);
        std::string value;
        
        // Get series ID
        std::getline(ss, value, ',');
        std::string series_id = trim(value);
        
        // Read values
        size_t value_count = 0;
        while (std::getline(ss, value, ',')) {
            value = trim(value);
            if (!value.empty()) {
                try {
                    // Remove any quotes
                    if (value.front() == '"') value = value.substr(1);
                    if (value.back() == '"') value.pop_back();
                    
                    // Convert to float
                    float val = std::stof(value);
                    if (idx < series.size()) {  // Safety check
                        series[idx].values.push_back(val);
                        total_values++;
                        value_count++;
                    }
                } catch (const std::exception& e) {
                    fprintf(stderr, "Warning: Failed to parse value: '%s' for series %s (column %zu)\n", 
                        value.c_str(), series_id.c_str(), value_count + 1);
                    parse_errors++;
                }
            }
        }
        
        if (value_count > 0 && (idx == 0 || idx == series.size()-1 || idx < 5)) {
            fprintf(stderr, "Series %s (type=%s): loaded %zu values\n", 
                series[idx].id.c_str(), series[idx].type.c_str(), value_count);
        }
        
        idx++;
    }
    
    fprintf(stderr, "\nData loading summary:\n");
    fprintf(stderr, "- Processed %zu series\n", idx);
    fprintf(stderr, "- Total values: %zu\n", total_values);
    fprintf(stderr, "- Parse errors: %zu\n", parse_errors);
    if (idx > 0) {
        fprintf(stderr, "- Average values per series: %.2f\n", total_values / (float)idx);
    }
    
    return total_values > 0;
}

void M4Dataset::normalize_series() {
    size_t normalized_count = 0;
    for (auto& s : series) {
        if (s.values.empty()) continue;
        
        // Calculate mean
        float sum = 0.0f;
        for (float v : s.values) {
            sum += v;
        }
        float mean = sum / s.values.size();
        
        // Calculate standard deviation
        float sq_sum = 0.0f;
        for (float v : s.values) {
            sq_sum += (v - mean) * (v - mean);
        }
        float std = std::sqrt(sq_sum / s.values.size());
        
        // Normalize values
        if (std > 0) {
            for (float& v : s.values) {
                v = (v - mean) / std;
            }
            normalized_count++;
        }
    }
    fprintf(stderr, "Normalized %zu series\n", normalized_count);
}

bool M4Dataset::prepare_training_data(ggml_opt_dataset_t dataset, const std::string& freq_filter) {
    struct ggml_tensor* data = ggml_opt_dataset_data(dataset);
    struct ggml_tensor* labels = ggml_opt_dataset_labels(dataset);
    
    float* data_ptr = ggml_get_data_f32(data);
    float* labels_ptr = ggml_get_data_f32(labels);
    
    size_t sample_idx = 0;
    const size_t max_samples = data->ne[1];  // batch dimension
    
    size_t filtered_series = 0;
    size_t too_short_series = 0;
    size_t used_series = 0;
    
    fprintf(stderr, "\nPreparing training data:\n");
    fprintf(stderr, "- Maximum samples: %zu\n", max_samples);
    fprintf(stderr, "- Sequence length: %d\n", TS_SEQUENCE_LENGTH);
    fprintf(stderr, "- Horizon: %d\n", TS_HORIZON);
    fprintf(stderr, "- Frequency filter: '%s'\n", freq_filter.c_str());
    
    for (const auto& s : series) {
        // Filter by frequency if specified
        if (!freq_filter.empty() && s.type != freq_filter) {
            filtered_series++;
            continue;
        }
        
        if (s.values.size() < TS_SEQUENCE_LENGTH + TS_HORIZON) {
            too_short_series++;
            fprintf(stderr, "Series %s too short: %zu values (need %d)\n", 
                s.id.c_str(), s.values.size(), TS_SEQUENCE_LENGTH + TS_HORIZON);
            continue;
        }
        
        // Create sliding window samples
        size_t windows_from_series = 0;
        for (size_t i = 0; i <= s.values.size() - (TS_SEQUENCE_LENGTH + TS_HORIZON); i++) {
            if (sample_idx >= max_samples) break;
            
            // Fill input sequence
            for (size_t j = 0; j < TS_SEQUENCE_LENGTH; j++) {
                data_ptr[sample_idx * TS_SEQUENCE_LENGTH + j] = s.values[i + j];
            }
            
            // Fill target values
            for (size_t j = 0; j < TS_HORIZON; j++) {
                labels_ptr[sample_idx * TS_HORIZON + j] = s.values[i + TS_SEQUENCE_LENGTH + j];
            }
            
            sample_idx++;
            windows_from_series++;
        }
        
        if (windows_from_series > 0) {
            used_series++;
        
        }
        
        if (sample_idx >= max_samples) break;
    }
    
    fprintf(stderr, "\nTraining data preparation results:\n");
    fprintf(stderr, "- Total series:     %zu\n", series.size());
    fprintf(stderr, "- Wrong frequency:  %zu\n", filtered_series);
    fprintf(stderr, "- Too short:        %zu (need %d values)\n", too_short_series, TS_SEQUENCE_LENGTH + TS_HORIZON);
    fprintf(stderr, "- Used series:      %zu\n", used_series);
    fprintf(stderr, "- Generated samples: %zu\n", sample_idx);
    
    return sample_idx > 0;
}