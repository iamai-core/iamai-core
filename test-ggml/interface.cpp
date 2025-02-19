#include "interface.h"
#include <cstdio>

M4Interface::M4Interface() : model(nullptr), is_initialized(false) {}

M4Interface::~M4Interface() {
    if (model != nullptr) {
        delete model;
    }
}

bool M4Interface::init(const std::string& info_file, const std::string& data_file, const std::string& frequency) {
    // Load M4 dataset
    if (!m4_data.load_info(info_file)) {
        fprintf(stderr, "Failed to load info file: %s\n", info_file.c_str());
        return false;
    }

    if (!m4_data.load_data(data_file)) {
        fprintf(stderr, "Failed to load data file: %s\n", data_file.c_str());
        return false;
    }

    // Normalize the data
    m4_data.normalize_series();
    current_frequency = frequency;
    is_initialized = true;

    return true;
}

bool M4Interface::train(const std::string& model_path, const std::string& backend) {
    if (!is_initialized) {
        fprintf(stderr, "Please initialize the interface first with init()\n");
        return false;
    }

    // Create dataset
    const size_t num_samples = 10000;  // Adjust based on your memory constraints
    ggml_opt_dataset_t dataset = ggml_opt_dataset_init(
        TS_SEQUENCE_LENGTH * TS_FEATURES,
        TS_HORIZON,
        num_samples,
        TS_BATCH_PHYSICAL
    );

    // Prepare training data
    if (!m4_data.prepare_training_data(dataset, current_frequency)) {
        fprintf(stderr, "Failed to prepare training data for frequency: %s\n", current_frequency.c_str());
        return false;
    }

    // Initialize model
    model = timeseries_model_init_random(backend, TS_BATCH_LOGICAL, TS_BATCH_PHYSICAL);

    // Build computational graph
    fprintf(stderr, "Building computational graph...\n");
    timeseries_model_build(*model);

    // Train the model
    fprintf(stderr, "Starting training for %s frequency...\n", current_frequency.c_str());
    const int nepochs = 50;
    const float validation_split = 0.2f;

    timeseries_model_train(
        *model,
        dataset,
        nepochs,
        validation_split
    );

    // Save the trained model
    timeseries_model_save(*model, model_path);
    fprintf(stderr, "Model saved to %s\n", model_path.c_str());

    return true;
}

bool M4Interface::evaluate(const std::string& model_path, const std::string& test_data_file, const std::string& backend) {
    if (!is_initialized) {
        fprintf(stderr, "Please initialize the interface first with init()\n");
        return false;
    }

    // Load test data
    M4Dataset test_data;
    if (!test_data.load_info(test_data_file)) {
        fprintf(stderr, "Failed to load test data file: %s\n", test_data_file.c_str());
        return false;
    }

    // Create dataset for testing
    const size_t num_test_samples = 1000;
    ggml_opt_dataset_t dataset = ggml_opt_dataset_init(
        TS_SEQUENCE_LENGTH * TS_FEATURES,
        TS_HORIZON,
        num_test_samples,
        TS_BATCH_PHYSICAL
    );

    // Prepare test data
    if (!test_data.prepare_training_data(dataset, current_frequency)) {
        fprintf(stderr, "Failed to prepare test data\n");
        return false;
    }

    // Load model if not already loaded
    if (model == nullptr) {
        model = timeseries_model_init_random(backend, TS_BATCH_LOGICAL, TS_BATCH_PHYSICAL);
        timeseries_model_load(*model, model_path);
        timeseries_model_build(*model);
    }

    // Evaluate model
    fprintf(stderr, "Evaluating model...\n");
    ggml_opt_result_t result = timeseries_model_eval(*model, dataset);

    // Calculate and print metrics
    double mse, rmse;
    ggml_opt_result_loss(result, &mse, &rmse);
    
    fprintf(stdout, "Test Results for %s frequency:\n", current_frequency.c_str());
    fprintf(stdout, "MSE:  %.6f\n", mse);
    fprintf(stdout, "RMSE: %.6f\n", rmse);

    ggml_opt_result_free(result);
    return true;
}