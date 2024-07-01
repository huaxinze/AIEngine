#pragma once

#include "status.h"
#include "model_config.pb.h"
#include "model_config.h"

namespace core {

/// Enumeration for the different backend types.
enum BackendType {
  BACKEND_TYPE_UNKNOWN = 0,
  BACKEND_TYPE_TENSORRT = 1,
  BACKEND_TYPE_TENSORFLOW = 2,
  BACKEND_TYPE_ONNXRUNTIME = 3,
  BACKEND_TYPE_PYTORCH = 4,
  BACKEND_TYPE_QNN = 5,
};

// Get version of a model from the path containing the model
/// definition file.
/// \param path The path to the model definition file.
/// \param version Returns the version.
/// \return The error status.
Status GetModelVersionFromPath(const std::string& path, int64_t* version);

/// Convert a model configuration JSON to the equivalent protobuf.
/// \param config The JSON model configuration.
/// \param config_version The model configuration will be returned in
/// a format matching this version. If the configuration cannot be
/// represented in the requested version's format then an error will
/// be returned.
/// \param protobuf Returns the equivalent protobuf.
/// \return The error status.
Status JsonToModelConfig(const std::string& json_config, 
                         const uint32_t config_version,
                         inference::ModelConfig* protobuf_config);

/// Validate that an output is specified correctly in a model
/// configuration.
/// \param io The model output.
/// \param max_batch_size The max batch size specified in model configuration.
/// \param platform The platform name
/// \return The error status. A non-OK status indicates the output
/// is not valid.
Status ValidateModelOutput(const inference::ModelOutput& io, 
                           int32_t max_batch_size,
                           const std::string& platform);

/// Validate that input is specified correctly in a model
/// configuration.
/// \param io The model input.
/// \param max_batch_size The max batch size specified in model configuration.
/// \param platform The platform name
/// \return The error status. A non-OK status indicates the input
/// is not valid.
Status ValidateModelInput(const inference::ModelInput& io, 
                          int32_t max_batch_size,
                          const std::string& platform);
                  
/// Validate that a model inputs and outputs are specified correctly.
/// \param config The model configuration to validate.
/// \return The error status. A non-OK status indicates the configuration
/// is not valid.
Status ValidateModelIOConfig(const inference::ModelConfig& config);

/// Validate that a model is specified correctly, except for model inputs
/// and outputs. ValidateModelIOConfig() should be called to
/// validate model inputs and outputs.
/// \param config The model configuration to validate.
/// \param min_compute_capability The minimum support CUDA compute
/// capability.
/// \return The error status. A non-OK status indicates the configuration
/// is not valid.
Status ValidateModelConfig(const inference::ModelConfig& config, 
                           const double min_compute_capability);

/// Get the BackendType value for a backend name.
/// \param backend_name The backend name.
/// \return The BackendType or BackendType::UNKNOWN if the platform string
/// is not recognized.
BackendType GetBackendType(const std::string& backend_name);

/// Get the BackendType value for a platform name.
/// \param platform_name The platform name.
/// \return The BackendType or BackendType::UNKNOWN if the platform string
/// is not recognized.
BackendType GetBackendTypeFromPlatform(const std::string& platform_name);

/// [FIXME] better formalize config normalization / validation
/// Validate instance group setting.
/// \param config The model configuration to validate.
/// \param min_compute_capability The minimum support CUDA compute
/// capability.
/// \return The error status. A non-OK status indicates the configuration
/// is not valid.
Status ValidateInstanceGroup(const inference::ModelConfig& config, 
                             const double min_compute_capability);

/// Load ModelConfig Form a prototext file.
/// \param path The path of the file.
/// \param msg Returns the model_config for the file.
/// \return Error status
Status LoadModelConfigFormTextProto(const std::string& path, 
                                    inference::ModelConfig* model_config);

/// Check if non instance group settings on the model configs are equivalent.
/// \param old_config The old model config.
/// \param new_config The new model config.
/// \return True if the model configs are equivalent in all non instance group
/// settings. False if they differ in non instance group settings.
bool EquivalentInNonInstanceGroupConfig(const inference::ModelConfig& old_config,
                                        const inference::ModelConfig& new_config);

/// Check if both model instance configs are equivalent. 'name' and 'count'
/// fields do not alter the functionality of the instance and hence excluded
/// from checking.
/// \param instance_config_lhs The left hand side instance config.
/// \param instance_config_rhs The right hand side instance config.
/// \return True if instance configs are the same without checking 'name' and
/// 'count' fields. False if they are different without checking the fields.
bool EquivalentInInstanceConfig(const inference::ModelInstanceGroup& instance_config_lhs,
                                const inference::ModelInstanceGroup& instance_config_rhs);

/// Obtain a signature identifying the instance config. 'name' and 'count'
/// fields do not alter the functionality of the instance and hence excluded
/// from altering the signature.
/// \param instance_config The instance config.
/// \return Signature identifying the instance config.
std::string InstanceConfigSignature(const inference::ModelInstanceGroup& instance_config);

} // namespace core