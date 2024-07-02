#include "model_config_utils.h"

#include "constants.h"
#include "file_utils.h"
#include "platform.h"
#include <set>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/message_differencer.h>

namespace core {

BackendType GetBackendTypeFromPlatform(const std::string& platform_name) {
  if (platform_name == kQnnPlatform) {
    return BackendType::BACKEND_TYPE_QNN;
  }
  if (platform_name == kPyTorchLibTorchPlatform) {
    return BackendType::BACKEND_TYPE_PYTORCH;
  }
  if (platform_name == kOnnxRuntimeOnnxPlatform) {
    return BackendType::BACKEND_TYPE_ONNXRUNTIME;
  }
  return BackendType::BACKEND_TYPE_UNKNOWN;
}

BackendType GetBackendType(const std::string& backend_name) {
  if (backend_name == kQnnBackend) {
    return BackendType::BACKEND_TYPE_QNN;
  }
  if (backend_name == kPyTorchBackend) {
    return BackendType::BACKEND_TYPE_PYTORCH;
  }
  if (backend_name == kOnnxRuntimeBackend) {
    return BackendType::BACKEND_TYPE_ONNXRUNTIME;
  }
  return BackendType::BACKEND_TYPE_UNKNOWN;
}

Status LoadModelConfigFormTextProto(const std::string& path, 
                                    inference::ModelConfig* model_config) {
  using namespace google::protobuf;
  std::string str;
  RETURN_IF_ERROR(ReadTextFile(path, &str));
  if (!TextFormat::ParseFromString(str, model_config)) {
    auto msg = "failed to read text proto from " + path;
    return Status(Status::Code::INTERNAL, msg);
  }
  return Status::Success;                                 
}

Status GetModelVersionFromPath(const std::string& path, int64_t* version) {
  auto version_dir = BaseName(path);
  try {
    *version = std::atoll(version_dir.c_str());
  } catch (...) {
    std::string msg = "unable to determine version from " + path;
    return Status(Status::Code::INTERNAL, msg);
  }
  return Status::Success;
}

Status JsonToModelConfig(const std::string& json_config, 
                         const uint32_t config_version,
                         inference::ModelConfig* protobuf_config) {
  if (config_version != 1) {
    std::string msg = "model configuration version " + 
                      std::to_string(config_version) + 
                      " not supported, supported versions are: 1";
    return Status(Status::Code::INVALID_ARG, msg);
  }
  using namespace google::protobuf;
  util::JsonParseOptions options;
  options.case_insensitive_enum_parsing = true;
  options.ignore_unknown_fields = false;
  auto err = util::JsonStringToMessage(json_config, 
                                       protobuf_config, 
                                       options);
  if (!err.ok()) {
    return Status(Status::Code::INVALID_ARG, std::string(err.message()));
  }
  return Status::Success;
}

Status ValidateModelIOConfig(const inference::ModelConfig& config) {
  Status status;
  for (const auto& input : config.input()) {
    status = ValidateModelInput(input, 
                                config.max_batch_size(), 
                                config.platform());
    if (!status.IsOk()) {
      auto msg = status.Message() + " for " + config.name();
      return Status(status.StatusCode(), msg);
    }
  }
  for (const auto& output : config.output()) {
    status = ValidateModelOutput(output, 
                                 config.max_batch_size(), 
                                 config.platform());
    if (!status.IsOk()) {
      auto msg = status.Message() + " for " + config.name();
      return Status(status.StatusCode(), msg);
    }
  }
  return Status::Success;
}

template <class ModelIO> Status ValidateIOShape(const ModelIO& io,
                       int32_t max_batch_size,
                       const std::string& message_prefix = "") {
  std::string msg = message_prefix;
  if (io.name().empty()) {
    msg += "must specify 'name'";
    return Status( Status::Code::INVALID_ARG, msg);
  }
  if (io.data_type() == inference::DataType::TYPE_INVALID) {
    msg += "must specify 'data_type'";
    return Status(Status::Code::INVALID_ARG, msg);
  }
  if (io.dims_size() == 0) {
    msg += "must specify 'dims'";
    return Status(Status::Code::INVALID_ARG, msg);
  }
  for (auto dim : io.dims()) {
    if ((dim < 1) && (dim != WILDCARD_DIM)) {
      msg += "dimension must be integer >= 1, or -1 to indicate a variable-size dimension.";
      return Status(Status::Code::INVALID_ARG, msg);
    }
  }
  return Status::Success;
}

Status ValidateModelInput(const inference::ModelInput& io, 
                          int32_t max_batch_size,
                          const std::string& platform) {
  RETURN_IF_ERROR(ValidateIOShape(io, max_batch_size, "model input "));
  if (((io.format() == inference::ModelInput::FORMAT_NHWC) ||
       (io.format() == inference::ModelInput::FORMAT_NCHW)) &&
      (io.dims_size() != 3)) {
    return Status(Status::Code::INVALID_ARG, "model input NHWC/NCHW require 3 dims");
  }
  return Status::Success;
}

Status ValidateModelOutput(const inference::ModelOutput& io, 
                           int32_t max_batch_size,
                           const std::string& platform) {
  RETURN_IF_ERROR(ValidateIOShape(io, max_batch_size, "model output "));
  return Status::Success;
}

Status ValidateModelConfig(const inference::ModelConfig& config, 
                           const double min_compute_capability) {
  if (config.name().empty()) {
    auto msg = "model configuration must specify 'name'";
    return Status(Status::Code::INVALID_ARG, msg);
  }
  if (config.platform().empty() && config.backend().empty()) {
    auto msg = "must specify 'platform' or 'backend' for '" + 
               config.name() + "'";
    return Status(Status::Code::INVALID_ARG, msg);
  }
  auto type = GetBackendType(config.backend());
  if ((type != BackendType::BACKEND_TYPE_UNKNOWN) &&
      (type != GetBackendTypeFromPlatform(config.platform()))) {
    auto msg = "unexpected 'platform' and 'backend', got: " +
               config.platform() +
               ", " +
               config.backend();
    return Status(Status::Code::INVALID_ARG, msg);
  }
  return Status::Success;
}

Status ValidateInstanceGroup(const inference::ModelConfig& config, 
                             const double min_compute_capability) {
  if (config.instance_group().size() == 0) {
    auto msg = "must specify one or more 'instance group's for " + 
               config.name();
    return Status(Status::Code::INVALID_ARG, msg);
  }
  std::set<int> supported_gpus;
  Status status = GetSupportedGPUs(&supported_gpus, 
                                   min_compute_capability);
  if (!status.IsOk()) {
    auto msg = "GetSupportedGPUs has been failed for " + 
               config.name();
    return Status(Status::Code::NOT_FOUND, msg);
  }
  for (const auto& group : config.instance_group()) {
    if (group.kind() == inference::ModelInstanceGroup::KIND_GPU) {
      if (group.gpus().size() == 0) {
        if (supported_gpus.size() == 0) {
          auto msg = "instance group " + 
                     group.name() + 
                     " of model " + 
                     config.name() +
                     " has kind KIND_GPU but no GPUs are available";
          return Status(Status::Code::INVALID_ARG, msg);
        } else {
          auto msg = "instance group " +
                     group.name() +
                     " of model " +
                     config.name() +
                     " has kind KIND_GPU but specifies no GPUs";
          return Status(Status::Code::INVALID_ARG, msg);
        }
      }
      for (const int32_t gid : group.gpus()) {
        if (supported_gpus.find(gid) == supported_gpus.end()) {
          std::string supported_gpus_str;
          for (const auto& cc : supported_gpus) {
            if (!supported_gpus_str.empty()) {
              supported_gpus_str += ", ";
            }
            supported_gpus_str += std::to_string(cc);
          }
          auto msg = "instance group " + 
                     group.name() +
                     " of model " +
                     config.name() +
                     " specifies invalid or unsupported gpu id " +
                     std::to_string(gid) +
                     ". GPUs with at least the minimum required CUDA compute "
                     "compatibility of " +
                     std::to_string(min_compute_capability) +
                     " are: " +
                    supported_gpus_str;
          return Status(Status::Code::INVALID_ARG, msg);
        }
      }
    } else if (group.kind() == inference::ModelInstanceGroup::KIND_CPU) {
      if (group.gpus().size() > 0) {
        auto msg = "instance group " + 
                   group.name() + 
                   " of model " + 
                   config.name() +
                   " has kind KIND_CPU but specifies one or more GPUs";
        return Status(Status::Code::INVALID_ARG, msg);
      }
    } else {
      auto msg = "instance group " + 
                 group.name() +
                 " of model " + 
                 config.name() +
                 " has unexpected kind KIND_AUTO";
      return Status(Status::Code::INTERNAL, msg);
    }
  }
  return Status::Success;
}

bool EquivalentInNonInstanceGroupConfig(const inference::ModelConfig& old_config,
                                        const inference::ModelConfig& new_config) {
  ::google::protobuf::util::MessageDifferencer pb_diff;
  pb_diff.IgnoreField(
    old_config.descriptor()->FindFieldByLowercaseName("instance_group"));
  return pb_diff.Compare(old_config, new_config);
}

bool EquivalentInInstanceConfig(const inference::ModelInstanceGroup& instance_config_lhs,
                                const inference::ModelInstanceGroup& instance_config_rhs) {
  ::google::protobuf::util::MessageDifferencer pb_diff;
  pb_diff.IgnoreField(
      instance_config_lhs.descriptor()->FindFieldByLowercaseName("name"));
  pb_diff.IgnoreField(
      instance_config_lhs.descriptor()->FindFieldByLowercaseName("count"));
  return pb_diff.Compare(instance_config_lhs, instance_config_rhs);
}

std::string InstanceConfigSignature(const inference::ModelInstanceGroup& instance_config) {
  inference::ModelInstanceGroup config = instance_config;
  *config.mutable_name() = "[Normalized]";
  config.set_count(1);
  return config.SerializeAsString();
}

/// Auto-complete the instance count based on instance kind and backend name.
/// \param group The instance group to set the count for.
/// \param backend The backend name to check against.
/// \return The error status.
Status SetDefaultInstanceCount(inference::ModelInstanceGroup* group, 
                               const std::string& backend) {
  group->set_count(1);
  // Backends opt into the default_cpu_instance_count since
  // some backends (pytorch, OpenVINO) don't perform well/have high overhead
  // when using multiple instances.
  const int default_cpu_instance_count = 2;
  bool use_default_cpu_instance_count =
      (backend == kTensorFlowBackend) || (backend == kOnnxRuntimeBackend);
  if (group->kind() == inference::ModelInstanceGroup::KIND_CPU &&
      use_default_cpu_instance_count) {
    group->set_count(default_cpu_instance_count);
  }
  return Status::Success;
}

Status NormalizeInstanceGroup(const double min_compute_capability,
                              const std::vector<inference::ModelInstanceGroup>& preferred_groups,
                              inference::ModelConfig* config) {
  // Instance group setting doesn't apply to ensemble
  if (config->has_ensemble_scheduling()) {
    return Status::Success;
  }
  // Creates a set of supported GPU device ids
  std::set<int> supported_gpus;
  // Get the total number of GPUs from the runtime library.
  Status status = GetSupportedGPUs(&supported_gpus, min_compute_capability);
  if (!status.IsOk()) {
    return status;
  }
  // Make sure there is at least one instance_group.
  if (config->instance_group().empty()) {
    inference::ModelInstanceGroup* group = config->add_instance_group();
    group->set_name(config->name());
    for (const auto& pg : preferred_groups) {
      // handle preferred GPU setting differently based on kind
      if (pg.kind() == inference::ModelInstanceGroup::KIND_GPU) {
        // Don't use preferred group with KIND_GPU if there is no GPU.
        if (supported_gpus.empty()) {
          continue;
        }
        // If preferred group sets GPUs, limit deployment onto those that
        // are also listed in supported gpus
        if (!pg.gpus().empty()) {
          for (const int32_t gid : pg.gpus()) {
            if (supported_gpus.find(gid) != supported_gpus.end()) {
              group->add_gpus(gid);
            }
          }
        }
      } else if (pg.kind() == inference::ModelInstanceGroup::KIND_AUTO) {
        // if AUTO, then set preferred GPU as is, to align with KIND_AUTO
        // deduction specified below
        for (const int32_t gid : pg.gpus()) {
          group->add_gpus(gid);
        }
      }
      group->set_kind(pg.kind());
      group->set_count(pg.count());
      // Found a valid preferred group.
      break;
    }
  }
  // Assign default name, kind and count to each instance group that
  // doesn't give those values explicitly. For KIND_GPU, set GPUs to
  // all available if not specified explicitly.
  size_t cnt = 0;
  for (auto& group : *config->mutable_instance_group()) {
    // Name
    if (group.name().empty()) {
      group.set_name(config->name() + "_" + std::to_string(cnt));
    }
    cnt++;
    // For KIND_AUTO... if there are no GPUs or if any of the listed
    // 'gpu's are not present, then use KIND_CPU.
    if (group.kind() == inference::ModelInstanceGroup::KIND_AUTO) {
      if (supported_gpus.empty()) {
        group.set_kind(inference::ModelInstanceGroup::KIND_CPU);
      } else {
        for (const int32_t gid : group.gpus()) {
          if (supported_gpus.find(gid) == supported_gpus.end()) {
            group.set_kind(inference::ModelInstanceGroup::KIND_CPU);
            break;
          }
        }
      }
      if (group.kind() == inference::ModelInstanceGroup::KIND_AUTO) {
        group.set_kind(inference::ModelInstanceGroup::KIND_GPU);
      }
    }
    // KIND is resolved at this point
    for (const auto& pg : preferred_groups) {
      if (group.kind() != pg.kind()) {
        continue;
      }
      // Limit the GPU setting within what is specified in the preferred group,
      // if no available GPU then skip to next preferred group
      if ((group.kind() == inference::ModelInstanceGroup::KIND_GPU) &&
          group.gpus().empty() && !pg.gpus().empty()) {
        for (const int32_t gid : pg.gpus()) {
          if (supported_gpus.find(gid) != supported_gpus.end()) {
            group.add_gpus(gid);
          }
        }
        if (group.gpus().empty()) {
          continue;
        }
      }
      if ((group.count() < 1) && (pg.count() > 0)) {
        group.set_count(pg.count());
      }
    }
    // Set Triton default if the fields are not set from preferred group
    // Count
    if (group.count() < 1) {
      RETURN_IF_ERROR(SetDefaultInstanceCount(&group, config->backend()));
    }
    // GPUs
    if ((group.kind() == inference::ModelInstanceGroup::KIND_GPU) &&
        (group.gpus().size() == 0)) {
      for (auto d : supported_gpus) {
        group.add_gpus(d);
      }
    }
  }
  return Status::Success;
}

} // namespace core