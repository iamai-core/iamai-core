#include "data.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <random>
#include <limits>

bool loadStockData(const std::string& filepath, StockData& data) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }
    
    // Clear existing data
    data.dates.clear();
    data.open.clear();
    data.high.clear();
    data.low.clear();
    data.close.clear();
    data.adj_close.clear();
    data.volume.clear();
    
    // Read header
    std::string line;
    if (!std::getline(file, line)) {
        std::cerr << "File is empty" << std::endl;
        return false;
    }
    
    // Read data lines
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        
        // Date
        if (!std::getline(ss, cell, ',')) break;
        data.dates.push_back(cell);
        
        // Open
        if (!std::getline(ss, cell, ',')) break;
        data.open.push_back(std::stof(cell));
        
        // High
        if (!std::getline(ss, cell, ',')) break;
        data.high.push_back(std::stof(cell));
        
        // Low
        if (!std::getline(ss, cell, ',')) break;
        data.low.push_back(std::stof(cell));
        
        // Close
        if (!std::getline(ss, cell, ',')) break;
        data.close.push_back(std::stof(cell));
        
        // Adj Close
        if (!std::getline(ss, cell, ',')) break;
        data.adj_close.push_back(std::stof(cell));
        
        // Volume
        if (!std::getline(ss, cell, ',')) break;
        data.volume.push_back(std::stof(cell));
    }
    
    file.close();
    
    if (data.dates.empty()) {
        std::cerr << "No data loaded" << std::endl;
        return false;
    }
    
    std::cout << "Loaded " << data.dates.size() << " data points" << std::endl;
    return true;
}

std::pair<std::vector<std::vector<float>>, std::vector<float>> 
createTimeSeriesDataset(const StockData& data, int window_size) {
    std::vector<std::vector<float>> X;
    std::vector<float> y;
    
    // We'll use several features: open, high, low, close, volume
    const int num_features = 5;
    
    // Make sure we have enough data
    size_t data_size = data.close.size();
    if (data_size <= static_cast<size_t>(window_size)) {
        std::cerr << "Not enough data for the specified window size" << std::endl;
        return {X, y};
    }
    
    // Create samples
    for (size_t i = window_size; i < data_size; ++i) {
        // Create a window of the past data
        std::vector<float> window_features;
        
        // For each day in the window
        for (int j = i - window_size; j < i; ++j) {
            // Add the features for this day
            window_features.push_back(data.open[j]);
            window_features.push_back(data.high[j]);
            window_features.push_back(data.low[j]);
            window_features.push_back(data.close[j]);
            window_features.push_back(data.volume[j]);
        }
        
        X.push_back(window_features);
        y.push_back(data.close[i]); // Predict the closing price
    }
    
    return {X, y};
}

void normalizeData(std::vector<std::vector<float>>& X, std::vector<float>& y) {
    if (X.empty() || y.empty()) {
        return;
    }
    
    // Normalize features (X)
    const int features_per_day = X[0].size() / (X[0].size() / 5); // Assuming 5 features per day
    
    // Find min and max for each feature
    std::vector<float> feature_min(features_per_day, std::numeric_limits<float>::max());
    std::vector<float> feature_max(features_per_day, std::numeric_limits<float>::lowest());
    
    for (const auto& sample : X) {
        for (size_t i = 0; i < sample.size(); ++i) {
            int feature_idx = i % features_per_day;
            feature_min[feature_idx] = std::min(feature_min[feature_idx], sample[i]);
            feature_max[feature_idx] = std::max(feature_max[feature_idx], sample[i]);
        }
    }
    
    // Normalize X
    for (auto& sample : X) {
        for (size_t i = 0; i < sample.size(); ++i) {
            int feature_idx = i % features_per_day;
            if (feature_max[feature_idx] > feature_min[feature_idx]) {
                sample[i] = (sample[i] - feature_min[feature_idx]) / 
                            (feature_max[feature_idx] - feature_min[feature_idx]);
            } else {
                sample[i] = 0.0f; // Handle constant features
            }
        }
    }
    
    // Normalize y (target)
    float y_min = *std::min_element(y.begin(), y.end());
    float y_max = *std::max_element(y.begin(), y.end());
    
    if (y_max > y_min) {
        for (auto& val : y) {
            val = (val - y_min) / (y_max - y_min);
        }
    }
}

void trainTestSplit(
    const std::vector<std::vector<float>>& X, 
    const std::vector<float>& y,
    std::vector<std::vector<float>>& X_train, 
    std::vector<float>& y_train,
    std::vector<std::vector<float>>& X_test, 
    std::vector<float>& y_test,
    float test_size) {
    
    if (X.empty() || y.empty() || X.size() != y.size()) {
        std::cerr << "Invalid input data for train-test split" << std::endl;
        return;
    }
    
    // For time series, we typically take the last portion as the test set
    size_t test_count = static_cast<size_t>(X.size() * test_size);
    size_t train_count = X.size() - test_count;
    
    // Resize output vectors
    X_train.resize(train_count);
    y_train.resize(train_count);
    X_test.resize(test_count);
    y_test.resize(test_count);
    
    // Split data
    for (size_t i = 0; i < train_count; ++i) {
        X_train[i] = X[i];
        y_train[i] = y[i];
    }
    
    for (size_t i = 0; i < test_count; ++i) {
        X_test[i] = X[train_count + i];
        y_test[i] = y[train_count + i];
    }
}