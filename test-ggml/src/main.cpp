#include "model.h"
#include "data.h"
#include <iostream>
#include <iomanip>

int main(int argc, char* argv[]) {
    // Hardcoded path to data directory
    const std::string data_dir = "C:/Side-Projects/IAMAI/iamai-chat/iamai-core/data";
    const std::string csv_path = data_dir + "/AAPL_1d.csv";
    
    std::cout << "Using data file: " << csv_path << std::endl;
    
    // Load stock data
    StockData stock_data;
    if (!loadStockData(csv_path, stock_data)) {
        std::cerr << "Failed to load stock data" << std::endl;
        return 1;
    }
    
    // Create time series dataset
    const int window_size = 10; // Look at the past 10 days to predict the next day
    auto [X, y] = createTimeSeriesDataset(stock_data, window_size);
    
    if (X.empty() || y.empty()) {
        std::cerr << "Failed to create dataset" << std::endl;
        return 1;
    }
    
    std::cout << "Created dataset with " << X.size() << " samples" << std::endl;
    
    // Normalize data
    normalizeData(X, y);
    
    // Split into training and testing sets
    std::vector<std::vector<float>> X_train, X_test;
    std::vector<float> y_train, y_test;
    trainTestSplit(X, y, X_train, y_train, X_test, y_test, 0.2);
    
    std::cout << "Training set size: " << X_train.size() << std::endl;
    std::cout << "Testing set size: " << X_test.size() << std::endl;
    
    // Create and initialize model
    const int input_size = X_train[0].size();
    const int hidden_size = 32;  // Adjust as needed
    const int output_size = 1;   // Predicting a single value (closing price)
    
    TimeSeriesModel model(input_size, hidden_size, output_size);
    model.initialize();  // Changed from if(!model.initialize())
    
    // Train the model
    std::cout << "Training model..." << std::endl;
    
    const int epochs = 1000;
    const float learning_rate = 0.01f;
    
    model.train(X_train, y_train, epochs, learning_rate);
    
    // Evaluate on test set
    std::vector<float> predictions = model.predict(X_test);
    
    // Calculate Mean Squared Error
    float mse = 0.0f;
    for (size_t i = 0; i < predictions.size(); ++i) {
        float error = predictions[i] - y_test[i];
        mse += error * error;
    }
    mse /= predictions.size();
    
    std::cout << "Test MSE: " << mse << std::endl;
    
    // Print some predictions vs actual values
    std::cout << "\nSample predictions:" << std::endl;
    std::cout << std::setw(15) << "Predicted" << std::setw(15) << "Actual" << std::endl;
    std::cout << std::string(30, '-') << std::endl;
    
    const int samples_to_show = std::min(10, static_cast<int>(predictions.size()));
    for (int i = 0; i < samples_to_show; ++i) {
        std::cout << std::fixed << std::setprecision(4);
        std::cout << std::setw(15) << predictions[i] << std::setw(15) << y_test[i] << std::endl;
    }
    
    // Save the model
    const std::string model_path = "aapl_model.bin";
    if (model.save(model_path)) {
        std::cout << "Model saved to " << model_path << std::endl;
    } else {
        std::cerr << "Failed to save model" << std::endl;
    }
    
    return 0;
}