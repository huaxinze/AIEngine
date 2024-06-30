#pragma once

#include <unordered_map>

#include "status.h"
#include "scheduler.h"
#include "model_config.h"

namespace core {

class Model {
 public:
  explicit Model(const double min_compute_capability, 
                 const std::string& model_dir,
                 const int64_t version, 
                 const inference::ModelConfig& config)
    : config_(config), 
      min_compute_capability_(min_compute_capability),
      version_(version), 
      required_input_count_(0), 
      model_dir_(model_dir),
      set_model_config_(false)
  {
  }

  virtual ~Model() {}

  // Get the name of model being served.
  const std::string& Name() const { return config_.name(); }

  // Get the version of model being served.
  int64_t Version() const { return version_; }

  // Get the configuration of model being served.
  const inference::ModelConfig& Config() const { return config_; }

  // Get the number of required inputs
  size_t RequiredInputCount() const { return required_input_count_; }

  // Get the model configuration for a named input.
  Status GetInput(const std::string& name, const inference::ModelInput** input) const;

  // Get the model configuration for a named output.
  Status GetOutput(const std::string& name, const inference::ModelOutput** output) const;

  uint64_t MaxPriorityLevel() const { return max_priority_level_; }

  uint64_t DefaultPriorityLevel() const { return default_priority_level_; }

  // Initialize the instance for Triton core usage
  Status Init(const bool is_config_provided);

  // Enqueue a request for execution. If Status::Success is returned
  // then the model has taken ownership of the request object and so
  // 'request' will be nullptr. If non-success is returned then the
  // caller still retains ownership of 'request'.
  Status Enqueue(std::unique_ptr<InferenceRequest>& request) {
    return scheduler_->Enqueue(request);
  }

  // Return the number of in-flight inferences.
  size_t InflightInferenceCount() {
    return scheduler_->InflightInferenceCount();
  }

  // Stop processing future requests unless they are considered as in-flight.
  void Stop() { scheduler_->Stop(); }

 protected:
  // Set the configuration of the model being served.
  Status SetModelConfig(const inference::ModelConfig& config);

  // Explicitly set the scheduler to use for inference requests to the
  // model. The scheduler can only be set once for a model.
  Status SetScheduler(std::unique_ptr<Scheduler> scheduler);

  // Configuration of the model.
  inference::ModelConfig config_;

  // The scheduler to use for this model.
  std::unique_ptr<Scheduler> scheduler_;

 private:
  // The minimum supported CUDA compute capability.
  const double min_compute_capability_;

  // Version of the model.
  int64_t version_;

  size_t required_input_count_;

    // Path to model
  std::string model_dir_;

  // The default priority level for the model.
  uint64_t default_priority_level_;

  // The largest priority value for the model.
  uint64_t max_priority_level_;

  // Whether or not model config has been set.
  bool set_model_config_;

  // Map from input name to the model configuration for that input.
  std::unordered_map<std::string, inference::ModelInput> input_map_;

  // Map from output name to the model configuration for that output.
  std::unordered_map<std::string, inference::ModelOutput> output_map_;
};

}