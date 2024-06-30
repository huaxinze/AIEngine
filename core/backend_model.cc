#pragma once

#include "backend_model.h"

namespace core {

Status BackendModel::Create(InferenceServer* server, 
                            const std::string& model_path,
                            const BackendCmdlineConfigMap& backend_cmdline_config_map,
                            const HostPolicyCmdlineConfigMap& host_policy_map,
                            const int64_t version, 
                            inference::ModelConfig model_config,
                            const bool is_config_provided, 
                            std::unique_ptr<BackendModel>* model) {
  return Status::Success;
}

// Merges the global backend configs with the specific
// backend configs.
Status BackendModel::ResolveBackendConfigs(const BackendCmdlineConfigMap& backend_cmdline_config_map,
                                           const std::string& backend_name, 
                                           BackendCmdlineConfig& config) {
  return Status::Success;
}

// Sets defaults for some backend configurations when none are specified on
// the command line.
Status BackendModel::SetBackendConfigDefaults(BackendCmdlineConfig& config) {
  return Status::Success;
}

// Get the search paths to the backend shared library.
std::vector<std::string> 
BackendModel::GetBackendLibrarySearchPaths(const std::string& model_path, 
                                           int64_t version,
                                           const std::string& backend_dir, 
                                           const std::string& backend_name) {
  return {};
}

// Get backend library directory and path, and search paths for the library
// and whether the backend is based on Python backend. The model configuration
// runtime field will be updated if left empty.
Status BackendModel::GetBackendLibraryProperties(const std::string& model_path, 
                                                 int64_t version,
                                                 const std::string& backend_dir, 
                                                 const std::string& backend_name,
                                                 inference::ModelConfig* model_config, 
                                                 bool* is_python_based_backend,
                                                 std::vector<std::string>* search_paths, 
                                                 std::string* backend_libdir,
                                                 std::string* backend_libpath) {
  return Status::Success;
}

// Get 'backend_libname', 'backend_libdir', 'backend_libpath' and
// 'is_python_based_backend' by searching for different possible backend
// library names on 'search_paths'. Searching for Python based backend
// runtime is limited to 'backend_dir'.
Status BackendModel::GetBackendRuntimeLibraryName(const std::string& backend_dir, 
                                                  const std::string& backend_name,
                                                  const std::vector<std::string>& search_paths,
                                                  std::string* backend_libname, 
                                                  std::string* backend_libdir,
                                                  std::string* backend_libpath, 
                                                  bool* is_python_based_backend) {
  return Status::Success;
}

// Search for 'backend_libname' on 'search_paths'. If found, the matching
// search path will be stored in 'backend_libdir' and the backend library path
// will be stored in 'backend_libpath'. If not found, 'backend_libpath' will
// be set to empty.
Status BackendModel::FindBackendLibraryPath(const std::vector<std::string>& search_paths,
                                            const std::string& backend_libname, 
                                            std::string* backend_libdir,
                                            std::string* backend_libpath) {
  return Status::Success;
}

// Assemble the C++ runtime library name.
std::string BackendModel::AssembleCPPRuntimeLibraryName(const std::string& backend_name) {
  return "";
}

BackendModel::~BackendModel() {
}

}