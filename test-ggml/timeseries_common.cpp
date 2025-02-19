#include "timeseries_common.h"
#include <random>
#include <fstream>
#include <cmath>
#include <thread>

TimeseriesModel::TimeseriesModel(const std::string & backend_name, const int nbatch_logical, const int nbatch_physical)
    : nbatch_logical(nbatch_logical), nbatch_physical(nbatch_physical) {
    // Initialize backend similar to MNIST example
    std::vector<ggml_backend_dev_t> devices;
    const int ncores = std::thread::hardware_concurrency();
    const int nthreads = std::min(ncores, (ncores + 4) / 2);

    if (!backend_name.empty()) {
        ggml_backend_dev_t dev = ggml_backend_dev_by_name(backend_name.c_str());
        if (!dev) {
            fprintf(stderr, "ERROR: backend %s not found\n", backend_name.c_str());
            exit(1);
        }
        
        ggml_backend_t backend = ggml_backend_dev_init(dev, nullptr);
        
        // Set thread count for CPU backend
        if (backend_name == "CPU") {
            // Using native CPU threading instead of GGML's CPU-specific functions
            // This provides a more generic solution that works with different backends
            #ifdef _OPENMP
            omp_set_num_threads(nthreads);
            #endif
        }
        
        backends.push_back(backend);
        devices.push_back(dev);
    }

    // Initialize contexts
    {
        const size_t size_meta = 1024*ggml_tensor_overhead();
        struct ggml_init_params params = {
            /*.mem_size   =*/ size_meta,
            /*.mem_buffer =*/ nullptr,
            /*.no_alloc   =*/ true,
        };
        ctx_static = ggml_init(params);
    }

    {
        const size_t size_meta = GGML_DEFAULT_GRAPH_SIZE*ggml_tensor_overhead() + 3*ggml_graph_overhead();
        struct ggml_init_params params = {
            /*.mem_size   =*/ size_meta,
            /*.mem_buffer =*/ nullptr,
            /*.no_alloc   =*/ true,
        };
        ctx_compute = ggml_init(params);
    }

    backend_sched = ggml_backend_sched_new(backends.data(), nullptr, backends.size(), GGML_DEFAULT_GRAPH_SIZE, false);
}

TimeseriesModel::~TimeseriesModel() {
    ggml_free(ctx_gguf);
    ggml_free(ctx_static);
    ggml_free(ctx_compute);

    ggml_backend_buffer_free(buf_gguf);
    ggml_backend_buffer_free(buf_static);
    ggml_backend_buffer_free(buf_compute);  // Added compute buffer cleanup
    
    ggml_backend_sched_free(backend_sched);
    for (ggml_backend_t backend : backends) {
        ggml_backend_free(backend);
    }
}

TimeseriesModel* timeseries_model_init_random(const std::string& backend, const int nbatch_logical, const int nbatch_physical) {
    auto* model = new TimeseriesModel(backend, nbatch_logical, nbatch_physical);
    
    // Initialize context
    {
        const size_t size_meta = 1024*ggml_tensor_overhead();
        struct ggml_init_params params = {
            /*.mem_size   =*/ size_meta,
            /*.mem_buffer =*/ nullptr,
            /*.no_alloc   =*/ true,
        };
        model->ctx_static = ggml_init(params);
        if (!model->ctx_static) {
            fprintf(stderr, "Failed to initialize static context\n");
            delete model;
            return nullptr;
        }
    }

    // Initialize random number generator
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<float> dist{0.0f, 0.1f};
    
    // Create tensors
    model->input = ggml_new_tensor_2d(model->ctx_static, GGML_TYPE_F32,
        TS_SEQUENCE_LENGTH * TS_FEATURES, model->nbatch_physical);
    
    model->input_weight = ggml_new_tensor_2d(model->ctx_static, GGML_TYPE_F32,
        TS_SEQUENCE_LENGTH * TS_FEATURES, TS_HIDDEN_SIZE);
    model->input_bias = ggml_new_tensor_1d(model->ctx_static, GGML_TYPE_F32,
        TS_HIDDEN_SIZE);
    
    model->hidden_weight = ggml_new_tensor_2d(model->ctx_static, GGML_TYPE_F32,
        TS_HIDDEN_SIZE, TS_HIDDEN_SIZE);
    model->hidden_bias = ggml_new_tensor_1d(model->ctx_static, GGML_TYPE_F32,
        TS_HIDDEN_SIZE);
    
    model->output_weight = ggml_new_tensor_2d(model->ctx_static, GGML_TYPE_F32,
        TS_HIDDEN_SIZE, TS_HORIZON);
    model->output_bias = ggml_new_tensor_1d(model->ctx_static, GGML_TYPE_F32,
        TS_HORIZON);

    // Allocate backend buffer
    model->buf_static = ggml_backend_alloc_ctx_tensors(model->ctx_static, model->backends[0]);
    if (model->buf_static == nullptr) {
        fprintf(stderr, "Failed to allocate backend buffer\n");
        ggml_free(model->ctx_static);
        delete model;
        return nullptr;
    }

    // Set tensor names
    ggml_set_name(model->input, "input");
    ggml_set_name(model->input_weight, "input_weight");
    ggml_set_name(model->input_bias, "input_bias");
    ggml_set_name(model->hidden_weight, "hidden_weight");
    ggml_set_name(model->hidden_bias, "hidden_bias");
    ggml_set_name(model->output_weight, "output_weight");
    ggml_set_name(model->output_bias, "output_bias");

    // Initialize weights with random values
    std::vector<ggml_tensor *> init_tensors = {
        model->input_weight, model->input_bias,
        model->hidden_weight, model->hidden_bias,
        model->output_weight, model->output_bias
    };

    for (ggml_tensor * t : init_tensors) {
        if (t == nullptr) {
            fprintf(stderr, "Error: Null tensor encountered\n");
            continue;
        }
    
        size_t num_elements = ggml_nelements(t);
        size_t num_bytes = ggml_nbytes(t);
        
        fprintf(stderr, "Initializing tensor: elements=%zu, bytes=%zu\n",
            num_elements, num_bytes);
    
        try {
            std::vector<float> weights(num_elements);
            for (float & w : weights) {
                w = dist(gen);
            }
            
            // Just call the function without checking return value
            ggml_backend_tensor_set(t, weights.data(), 0, num_bytes);
            
            fprintf(stderr, "Successfully initialized tensor %s\n", ggml_get_name(t));
        } catch (const std::exception& e) {
            fprintf(stderr, "Exception during tensor initialization: %s\n", e.what());
        }
    }

    return model;
}

void timeseries_model_build(TimeseriesModel& model) {
    // First, free any existing compute buffer
    if (model.buf_compute) {
        ggml_backend_buffer_free(model.buf_compute);
        model.buf_compute = nullptr;
    }

    // Clear the compute context
    ggml_free(model.ctx_compute);
    {
        const size_t size_meta = GGML_DEFAULT_GRAPH_SIZE*ggml_tensor_overhead() + 3*ggml_graph_overhead();
        struct ggml_init_params params = {
            /*.mem_size   =*/ size_meta,
            /*.mem_buffer =*/ nullptr,
            /*.no_alloc   =*/ true,
        };
        model.ctx_compute = ggml_init(params);
    }

    // Set computational graph parameters
    ggml_set_param(model.ctx_compute, model.input_weight);
    ggml_set_param(model.ctx_compute, model.input_bias);
    ggml_set_param(model.ctx_compute, model.hidden_weight);
    ggml_set_param(model.ctx_compute, model.hidden_bias);
    ggml_set_param(model.ctx_compute, model.output_weight);
    ggml_set_param(model.ctx_compute, model.output_bias);

    // Build forward computation graph
    struct ggml_tensor* cur = model.input;
    
    // Input layer
    cur = ggml_mul_mat(model.ctx_compute, model.input_weight, cur);
    cur = ggml_add(model.ctx_compute, cur, model.input_bias);
    cur = ggml_relu(model.ctx_compute, cur);
    
    // Hidden layer
    cur = ggml_mul_mat(model.ctx_compute, model.hidden_weight, cur);
    cur = ggml_add(model.ctx_compute, cur, model.hidden_bias);
    cur = ggml_relu(model.ctx_compute, cur);
    
    // Output layer
    model.output = ggml_mul_mat(model.ctx_compute, model.output_weight, cur);
    model.output = ggml_add(model.ctx_compute, model.output, model.output_bias);
    
    ggml_set_name(model.output, "output");

    // Allocate compute buffer
    model.buf_compute = ggml_backend_alloc_ctx_tensors(model.ctx_compute, model.backends[0]);
}

ggml_opt_result_t timeseries_model_eval(TimeseriesModel& model, ggml_opt_dataset_t dataset) {
    ggml_opt_params params = ggml_opt_default_params(model.backend_sched, 
        model.ctx_compute, model.input, model.output, GGML_OPT_LOSS_TYPE_MSE);
    params.build_type = GGML_OPT_BUILD_TYPE_FORWARD;
    
    ggml_opt_context_t opt_ctx = ggml_opt_init(params);
    ggml_opt_result_t result = ggml_opt_result_init();

    const int64_t t_start_us = ggml_time_us();
    ggml_opt_epoch(opt_ctx, dataset, nullptr, result, 0, nullptr, nullptr);
    const int64_t t_end_us = ggml_time_us();

    fprintf(stderr, "Evaluation took %.2f ms\n", (t_end_us - t_start_us) / 1000.0f);

    ggml_opt_free(opt_ctx);
    return result;
}

void timeseries_model_train(TimeseriesModel& model, ggml_opt_dataset_t dataset, 
    const int nepoch, const float val_split) {
    
    ggml_opt_fit(model.backend_sched, model.ctx_compute, model.input, model.output,
        dataset, GGML_OPT_LOSS_TYPE_MSE, ggml_opt_get_default_optimizer_params,
        nepoch, model.nbatch_logical, val_split, false);
}

void timeseries_model_save(TimeseriesModel& model, const std::string& fname) {
    fprintf(stderr, "Saving model to %s\n", fname.c_str());
    // TODO: Implement model saving
}

void timeseries_model_load(TimeseriesModel& model, const std::string& fname) {
    fprintf(stderr, "Loading model from %s\n", fname.c_str());
    // TODO: Implement model loading
}