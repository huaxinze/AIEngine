#include "model.h"
#include "model_config_utils.h"

namespace core {

Status Model::SetModelConfig(const inference::ModelConfig& config) {
  config_ = config;
  set_model_config_ = true;
  return Status::Success;
}

Status Model::SetScheduler(std::unique_ptr<Scheduler> scheduler) {
  if (scheduler_ != nullptr) {
    auto msg = "Attempt to change scheduler not allowed";
    return Status(Status::Code::INTERNAL, msg);
  }
  scheduler_ = std::move(scheduler);
  return Status::Success;
}

Status Model::GetInput(const std::string& name, 
                       const inference::ModelInput** input) const {
  const auto itr = input_map_.find(name);
  if (itr == input_map_.end()) {
    auto msg = "unexpected inference input '" +
               name +
               "' for model '" + 
               Name() + 
               "'";
    return Status(Status::Code::INVALID_ARG, msg);
  }
  *input = &itr->second;
  return Status::Success;
}

Status Model::GetOutput(const std::string& name, 
                        const inference::ModelOutput** output) const {
  const auto itr = output_map_.find(name);
  if (itr == output_map_.end()) {
    auto msg = "unexpected inference output '" +
                name +
                "' for model '" + 
                Name() + 
                "'";
    return Status(Status::Code::INVALID_ARG, msg);
  }
  *output = &itr->second;
  return Status::Success;
}

Status Model::Init(const bool is_config_provided) {
  if (!set_model_config_ && !is_config_provided) {
    auto msg = "model configuration is not provided for model '" + 
               Name() + 
               "'";
    return Status(Status::Code::NOT_FOUND, msg);
  }
  RETURN_IF_ERROR(ValidateModelConfig(config_, min_compute_capability_));
  RETURN_IF_ERROR(ValidateModelIOConfig(config_));
  // Initialize the input map
  for (const auto& io : config_.input()) {
    input_map_.insert(std::make_pair(io.name(), io));
    if (!io.optional()) {
      ++required_input_count_;
    }
  }
  // Initialize the output map
  for (const auto& io : config_.output()) {
    output_map_.insert(std::make_pair(io.name(), io));
  }
  return Status::Success;
}

}