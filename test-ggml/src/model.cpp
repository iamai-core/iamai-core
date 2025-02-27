#include "model.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <algorithm>

TimeSeriesModel::TimeSeriesModel(int input_size, int hidden_size, int output_size)
    : input_size_(input_size), 
      hidden_size_(hidden_size), 
      output_size_(output_size) {
}

TimeSeriesModel::~TimeSeriesModel() {
}

void TimeSeriesModel::initialize() {
    // Resize weight vectors
    weights_ih_.resize(input_size_ * hidden_size_);
    weights_ho_.resize(hidden_size_ * output_size_);
    bias_h_.resize(hidden_size_);
    bias_o_.resize(output_size_);
    
    // Initialize weights with small random values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-0.1f, 0.1f);
    
    for (int i = 0; i < input_size_ * hidden_size_; ++i) {
        weights_ih_[i] = dist(gen);
    }
    
    for (int i = 0; i < hidden_size_ * output_size_; ++i) {
        weights_ho_[i] = dist(gen);
    }
    
    // Initialize biases to zero
    std::fill(bias_h_.begin(), bias_h_.end(), 0.0f);
    std::fill(bias_o_.begin(), bias_o_.end(), 0.0f);
    
    std::cout << "Model initialized with "
              << input_size_ << " input features, "
              << hidden_size_ << " hidden neurons, and "
              << output_size_ << " output neurons" << std::endl;
}

std::vector<float> TimeSeriesModel::forward(const std::vector<float>& input) {
    // Check input size
    if (input.size() != static_cast<size_t>(input_size_)) {
        std::cerr << "Input size mismatch in forward pass" << std::endl;
        return std::vector<float>(output_size_, 0.0f);
    }
    
    // Compute hidden layer activations
    std::vector<float> hidden(hidden_size_, 0.0f);
    for (int h = 0; h < hidden_size_; ++h) {
        hidden[h] = bias_h_[h];  // Start with bias
        for (int i = 0; i < input_size_; ++i) {
            hidden[h] += input[i] * weights_ih_[i * hidden_size_ + h];
        }
        // Apply tanh activation
        hidden[h] = std::tanh(hidden[h]);
    }
    
    // Compute output layer
    std::vector<float> output(output_size_, 0.0f);
    for (int o = 0; o < output_size_; ++o) {
        output[o] = bias_o_[o];  // Start with bias
        for (int h = 0; h < hidden_size_; ++h) {
            output[o] += hidden[h] * weights_ho_[h * output_size_ + o];
        }
    }
    
    return output;
}

float TimeSeriesModel::compute_loss(const std::vector<std::vector<float>>& X, const std::vector<float>& y) {
    float loss = 0.0f;
    const int batch_size = X.size();
    
    for (int i = 0; i < batch_size; ++i) {
        std::vector<float> pred = forward(X[i]);
        float error = pred[0] - y[i];
        loss += error * error;
    }
    loss /= batch_size;
    
    return loss;
}

bool TimeSeriesModel::train(const std::vector<std::vector<float>>& X, 
                           const std::vector<float>& y,
                           int epochs, 
                           float learning_rate) {
    if (X.empty() || y.empty() || X.size() != y.size()) {
        std::cerr << "Invalid input data" << std::endl;
        return false;
    }
    
    const int batch_size = X.size();
    
    // Training loop
    for (int epoch = 0; epoch < epochs; ++epoch) {
        // Accumulate gradients over the batch
        std::vector<float> grad_w_ih(input_size_ * hidden_size_, 0.0f);
        std::vector<float> grad_w_ho(hidden_size_ * output_size_, 0.0f);
        std::vector<float> grad_b_h(hidden_size_, 0.0f);
        std::vector<float> grad_b_o(output_size_, 0.0f);
        
        // Process each sample in the batch
        for (int sample = 0; sample < batch_size; ++sample) {
            const auto& input = X[sample];
            const float target = y[sample];
            
            // Forward pass
            std::vector<float> hidden(hidden_size_, 0.0f);
            std::vector<float> hidden_pre_activation(hidden_size_, 0.0f);
            
            // Input to hidden layer
            for (int h = 0; h < hidden_size_; ++h) {
                hidden_pre_activation[h] = bias_h_[h];  // Start with bias
                for (int i = 0; i < input_size_; ++i) {
                    hidden_pre_activation[h] += input[i] * weights_ih_[i * hidden_size_ + h];
                }
                // Apply tanh activation
                hidden[h] = std::tanh(hidden_pre_activation[h]);
            }
            
            // Hidden to output layer
            std::vector<float> output(output_size_, 0.0f);
            for (int o = 0; o < output_size_; ++o) {
                output[o] = bias_o_[o];  // Start with bias
                for (int h = 0; h < hidden_size_; ++h) {
                    output[o] += hidden[h] * weights_ho_[h * output_size_ + o];
                }
            }
            
            // Compute error (output - target)
            std::vector<float> output_error(output_size_, 0.0f);
            for (int o = 0; o < output_size_; ++o) {
                output_error[o] = output[o] - target;
            }
            
            // Backpropagation
            // Output layer gradients
            for (int o = 0; o < output_size_; ++o) {
                // Bias gradient
                grad_b_o[o] += output_error[o];
                
                // Weights gradient
                for (int h = 0; h < hidden_size_; ++h) {
                    grad_w_ho[h * output_size_ + o] += output_error[o] * hidden[h];
                }
            }
            
            // Hidden layer gradients
            for (int h = 0; h < hidden_size_; ++h) {
                float hidden_error = 0.0f;
                for (int o = 0; o < output_size_; ++o) {
                    hidden_error += output_error[o] * weights_ho_[h * output_size_ + o];
                }
                
                // Apply tanh derivative: 1 - tanh^2(x)
                float tanh_deriv = 1.0f - hidden[h] * hidden[h];
                hidden_error *= tanh_deriv;
                
                // Bias gradient
                grad_b_h[h] += hidden_error;
                
                // Weights gradient
                for (int i = 0; i < input_size_; ++i) {
                    grad_w_ih[i * hidden_size_ + h] += hidden_error * input[i];
                }
            }
        }
        
        // Update weights with average gradients
        const float scale = learning_rate / batch_size;
        
        // Update input-to-hidden weights
        for (int i = 0; i < input_size_ * hidden_size_; ++i) {
            weights_ih_[i] -= scale * grad_w_ih[i];
        }
        
        // Update hidden-to-output weights
        for (int i = 0; i < hidden_size_ * output_size_; ++i) {
            weights_ho_[i] -= scale * grad_w_ho[i];
        }
        
        // Update biases
        for (int i = 0; i < hidden_size_; ++i) {
            bias_h_[i] -= scale * grad_b_h[i];
        }
        
        for (int i = 0; i < output_size_; ++i) {
            bias_o_[i] -= scale * grad_b_o[i];
        }
        
        // Compute and report loss occasionally
        if (epoch % 100 == 0 || epoch == epochs - 1) {
            float loss = compute_loss(X, y);
            std::cout << "Epoch " << epoch << ", Loss: " << loss << std::endl;
        }
    }
    
    return true;
}

std::vector<float> TimeSeriesModel::predict(const std::vector<std::vector<float>>& X) {
    const int batch_size = X.size();
    std::vector<float> predictions(batch_size);
    
    for (int i = 0; i < batch_size; ++i) {
        std::vector<float> output = forward(X[i]);
        predictions[i] = output[0];  // Assuming output_size_ = 1
    }
    
    return predictions;
}

bool TimeSeriesModel::save(const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for saving model: " << path << std::endl;
        return false;
    }
    
    // Save model architecture
    file.write((char*)&input_size_, sizeof(input_size_));
    file.write((char*)&hidden_size_, sizeof(hidden_size_));
    file.write((char*)&output_size_, sizeof(output_size_));
    
    // Save weights and biases
    file.write((char*)weights_ih_.data(), input_size_ * hidden_size_ * sizeof(float));
    file.write((char*)weights_ho_.data(), hidden_size_ * output_size_ * sizeof(float));
    file.write((char*)bias_h_.data(), hidden_size_ * sizeof(float));
    file.write((char*)bias_o_.data(), output_size_ * sizeof(float));
    
    file.close();
    return true;
}

bool TimeSeriesModel::load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for loading model: " << path << std::endl;
        return false;
    }
    
    // Load model architecture
    int loaded_input_size, loaded_hidden_size, loaded_output_size;
    file.read((char*)&loaded_input_size, sizeof(loaded_input_size));
    file.read((char*)&loaded_hidden_size, sizeof(loaded_hidden_size));
    file.read((char*)&loaded_output_size, sizeof(loaded_output_size));
    
    // Check if dimensions match
    if (loaded_input_size != input_size_ || 
        loaded_hidden_size != hidden_size_ || 
        loaded_output_size != output_size_) {
        std::cerr << "Model architecture mismatch" << std::endl;
        return false;
    }
    
    // Resize vectors
    weights_ih_.resize(input_size_ * hidden_size_);
    weights_ho_.resize(hidden_size_ * output_size_);
    bias_h_.resize(hidden_size_);
    bias_o_.resize(output_size_);
    
    // Load weights and biases
    file.read((char*)weights_ih_.data(), input_size_ * hidden_size_ * sizeof(float));
    file.read((char*)weights_ho_.data(), hidden_size_ * output_size_ * sizeof(float));
    file.read((char*)bias_h_.data(), hidden_size_ * sizeof(float));
    file.read((char*)bias_o_.data(), output_size_ * sizeof(float));
    
    file.close();
    return true;
}