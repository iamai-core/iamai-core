#pragma once

#include <vector>
#include <string>

// Pure C++ implementation of a simple neural network for time series prediction
class TimeSeriesModel {
public:
    TimeSeriesModel(int input_size, int hidden_size, int output_size);
    ~TimeSeriesModel();

    // Initialize the model with random weights
    void initialize();

    // Train the model using simple gradient descent
    bool train(const std::vector<std::vector<float>>& X, 
               const std::vector<float>& y,
               int epochs, 
               float learning_rate);

    // Make predictions
    std::vector<float> predict(const std::vector<std::vector<float>>& X);

    // Save/load model
    bool save(const std::string& path);
    bool load(const std::string& path);

private:
    // Model dimensions
    int input_size_;
    int hidden_size_;
    int output_size_;
    
    // Model parameters (stored as standard C++ vectors)
    std::vector<float> weights_ih_;  // Input to hidden weights [input_size x hidden_size]
    std::vector<float> weights_ho_;  // Hidden to output weights [hidden_size x output_size]
    std::vector<float> bias_h_;      // Hidden bias [hidden_size]
    std::vector<float> bias_o_;      // Output bias [output_size]
    
    // Forward pass for a single sample
    std::vector<float> forward(const std::vector<float>& input);
    
    // Compute loss for current weights
    float compute_loss(const std::vector<std::vector<float>>& X, const std::vector<float>& y);
};