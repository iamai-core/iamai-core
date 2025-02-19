#pragma once

#include <ggml.h>
#include <ggml-alloc.h>
#include <ggml-backend.h>
#include <ggml-opt.h>

#include <vector>
#include <string>

// Configuration for time series data
#define TS_SEQUENCE_LENGTH 10  // Number of past values to consider
#define TS_FEATURES       1   // Number of features per timestep
#define TS_HORIZON       1    // Number of future values to predict
#define TS_HIDDEN_SIZE   64   // Size of hidden layer

// Batch sizes for training
#define TS_BATCH_LOGICAL  100
#define TS_BATCH_PHYSICAL 50

// Define loss types if not defined by GGML
#ifndef GGML_OPT_LOSS_TYPE_MSE
#define GGML_OPT_LOSS_TYPE_MSE ((enum ggml_opt_loss_type)2)
#endif

class TimeseriesModel {
public:
    std::string arch;
    ggml_backend_sched_t backend_sched;
    std::vector<ggml_backend_t> backends;
    const int nbatch_logical;
    const int nbatch_physical;

    // Input tensor
    struct ggml_tensor* input = nullptr;     // [sequence_length, batch_size]
    
    // Model parameters
    struct ggml_tensor* input_weight = nullptr;  // [sequence_length, hidden_size]
    struct ggml_tensor* input_bias = nullptr;    // [hidden_size]
    struct ggml_tensor* hidden_weight = nullptr; // [hidden_size, hidden_size]
    struct ggml_tensor* hidden_bias = nullptr;   // [hidden_size]
    struct ggml_tensor* output_weight = nullptr; // [hidden_size, horizon]
    struct ggml_tensor* output_bias = nullptr;   // [horizon]
    
    // Output tensor
    struct ggml_tensor* output = nullptr;    // [horizon, batch_size]

    // GGML contexts and buffers
    struct ggml_context* ctx_gguf = nullptr;
    struct ggml_context* ctx_static = nullptr;
    struct ggml_context* ctx_compute = nullptr;
    
    ggml_backend_buffer_t buf_gguf = nullptr;
    ggml_backend_buffer_t buf_static = nullptr;
    ggml_backend_buffer_t buf_compute = nullptr;  // Added compute buffer

    // Constructor and destructor
    TimeseriesModel(const std::string& backend_name, const int nbatch_logical, const int nbatch_physical);
    ~TimeseriesModel();

    // Prevent copying
    TimeseriesModel(const TimeseriesModel&) = delete;
    TimeseriesModel& operator=(const TimeseriesModel&) = delete;
};

// Function declarations
TimeseriesModel* timeseries_model_init_random(const std::string& backend, const int nbatch_logical, const int nbatch_physical);
void timeseries_model_build(TimeseriesModel& model);
ggml_opt_result_t timeseries_model_eval(TimeseriesModel& model, ggml_opt_dataset_t dataset);
void timeseries_model_train(TimeseriesModel& model, ggml_opt_dataset_t dataset, const int nepoch, const float val_split);
void timeseries_model_save(TimeseriesModel& model, const std::string& fname);
void timeseries_model_load(TimeseriesModel& model, const std::string& fname);