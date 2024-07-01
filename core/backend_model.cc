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
  const auto& global_itr = 
    backend_cmdline_config_map.find(std::string());
  const auto& specific_itr = 
    backend_cmdline_config_map.find(backend_name);
  std::map<std::string, std::string> lconfig;
  if (global_itr != backend_cmdline_config_map.end()) {
    // Accumulate all global settings
    for (auto& setting : global_itr->second) {
      lconfig[setting.first] = setting.second;
    }
  }
  if (specific_itr != backend_cmdline_config_map.end()) {
    // Accumulate backend specific settings and override
    // global settings with specific configs if needed
    for (auto& setting : specific_itr->second) {
      lconfig[setting.first] = setting.second;
    }
  }
  for (auto& final_setting : lconfig) {
    config.emplace_back(final_setting);
  }
  return Status::Success;
}

const std::unordered_map<std::string, std::string> backend_config_defaults(
  {{"default-max-batch-size", "4"}});

// Sets defaults for some backend configurations when none are specified on
// the command line.
Status BackendModel::SetBackendConfigDefaults(BackendCmdlineConfig& config) {
  auto backend_config_defaults_copy = backend_config_defaults;
  for (auto& setting : config) {
    if (setting.first.compare("default-max-batch-size") == 0) {
      backend_config_defaults_copy.erase(setting.first);
    }
    if (backend_config_defaults_copy.empty()) {
      break;
    }
  }
  // Anything left should be added to the config
  for (const auto& default_setting : backend_config_defaults_copy) {
    config.push_back(std::make_pair(default_setting.first, default_setting.second));
  }
  return Status::Success;
}

// Get the search paths to the backend shared library.
std::vector<std::string> 
BackendModel::GetBackendLibrarySearchPaths(const std::string& model_path, 
                                           int64_t version,
                                           const std::string& backend_dir, 
                                           const std::string& backend_name) {
  const auto version_path = JoinPath({model_path, std::to_string(version)});
  const auto backend_path = JoinPath({backend_dir, backend_name});
  std::vector<std::string> search_paths = { version_path, model_path, backend_path};
  return search_paths;
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
  // backend_libname
  std::string backend_libname = model_config->runtime();
  if (backend_libname.empty()) {
    // Get backend library name, directory and path by backend_name and *search_paths
    RETURN_IF_ERROR(GetBackendRuntimeLibraryName(backend_dir, 
                                                 backend_name, 
                                                 *search_paths, 
                                                 &backend_libname,
                                                 backend_libdir, 
                                                 backend_libpath, 
                                                 is_python_based_backend));
    model_config->set_runtime(backend_libname);
    return Status::Success;
  }
  // backend_libpath
  std::string cpp_backend_libname = backend_libname;
  // Get backend library directory and path by backend_name and *search_paths
  RETURN_IF_ERROR(FindBackendLibraryPath(*search_paths, 
                                         cpp_backend_libname, 
                                         backend_libdir, 
                                         backend_libpath));
  if (backend_libpath->empty()) {
    std::string search_paths_str = "";
    for (const auto& path : *search_paths) {
      search_paths_str += "'" + path + "' ";
    }
    auto msg = "unable to find backend library '" +
               cpp_backend_libname + 
               "' for model '" + 
               model_config->name() + 
               "', searched: " + search_paths_str;
    return Status(Status::Code::INVALID_ARG, msg);
  }
  if (IsChildPathEscapingParentPath(*backend_libpath /* child_path */,
                                    *backend_libdir /* parent_path */)) {
    auto msg = "backend library name '" +
               cpp_backend_libname +
               "' escapes backend directory '" + 
               *backend_libdir +
               "', for model '" + 
               model_config->name() +
               "', check model config runtime field";
    return Status(Status::Code::INVALID_ARG, msg);
  }
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
  // Try C++ runtime
  *backend_libname = AssembleCPPRuntimeLibraryName(backend_name);
  RETURN_IF_ERROR(FindBackendLibraryPath(search_paths, 
                                         *backend_libname, 
                                         backend_libdir, 
                                         backend_libpath));
  if (!backend_libpath->empty()) {
    return Status::Success;
  }
  // Cannot find runtime
  auto msg = "unable to find backend library for backend '" + backend_name +
             "', try specifying runtime on the model configuration.";
  return Status(Status::Code::INVALID_ARG, msg);
}

// Search for 'backend_libname' on 'search_paths'. If found, the matching
// search path will be stored in 'backend_libdir' and the backend library path
// will be stored in 'backend_libpath'. If not found, 'backend_libpath' will
// be set to empty.
Status BackendModel::FindBackendLibraryPath(const std::vector<std::string>& search_paths,
                                            const std::string& backend_libname, 
                                            std::string* backend_libdir,
                                            std::string* backend_libpath) {
  backend_libpath->clear();
  for (const auto& path : search_paths) {
    const auto full_path = JoinPath({path, backend_libname});
    bool exists = false;
    RETURN_IF_ERROR(FileExists(full_path, &exists));
    if (exists) {
      *backend_libdir = path;
      *backend_libpath = full_path;
      break;
    }
  }
  return Status::Success;
}

// Assemble the C++ runtime library name.
std::string BackendModel::AssembleCPPRuntimeLibraryName(const std::string& backend_name) {
#ifdef _WIN32
  return "triton_" + backend_name + ".dll";
#else
  return "libtriton_" + backend_name + ".so";
#endif
}

BackendModel::~BackendModel() {
}

}