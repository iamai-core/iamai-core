#pragma once

#include <vector>
#include <string>
#include <utility>

// Structure to hold yfinance data
struct StockData {
    std::vector<std::string> dates;
    std::vector<float> open;
    std::vector<float> high;
    std::vector<float> low;
    std::vector<float> close;
    std::vector<float> adj_close;
    std::vector<float> volume;
};

// Load stock data from a CSV file
bool loadStockData(const std::string& filepath, StockData& data);

// Create time series dataset from stock data
// window_size: Number of previous days to use as features
// returns: pair of (X, y) where X is the input features and y is the target values
std::pair<std::vector<std::vector<float>>, std::vector<float>> 
createTimeSeriesDataset(const StockData& data, int window_size);

// Normalize data to [0, 1] range
void normalizeData(std::vector<std::vector<float>>& X, std::vector<float>& y);

// Split data into training and testing sets
void trainTestSplit(
    const std::vector<std::vector<float>>& X, 
    const std::vector<float>& y,
    std::vector<std::vector<float>>& X_train, 
    std::vector<float>& y_train,
    std::vector<std::vector<float>>& X_test, 
    std::vector<float>& y_test,
    float test_size = 0.2);