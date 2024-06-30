#pragma once

#include <memory>
#include <string>

#include "status.h"
#include "model.h"
#include "backend.h"
#include "constants.h"
#include "file_utils.h"
#include "backend_model.h"

namespace core {

class InferenceServer;

class BackendModel : public Model {
 public:
  static Status Create(InferenceServer* server, 
                       const std::string& model_path,
                       const BackendCmdlineConfigMap& backend_cmdline_config_map,
                       const HostPolicyCmdlineConfigMap& host_policy_map,
                       const int64_t version, 
                       inference::ModelConfig model_config,
                       const bool is_config_provided, 
                       std::unique_ptr<BackendModel>* model);
  ~BackendModel();

 private:
  DISALLOW_COPY_AND_ASSIGN(BackendModel);

  BackendModel(InferenceServer* server,
              const std::shared_ptr<LocalizedPath>& localized_model_dir,
              const std::shared_ptr<Backend>& backend,
              const double min_compute_capability, 
              const int64_t version,
              const inference::ModelConfig& config, 
              const bool auto_complete_config,
              const BackendCmdlineConfigMap& backend_cmdline_config_map,
              const HostPolicyCmdlineConfigMap& host_policy_map)
    : Model(min_compute_capability, localized_model_dir->Path(), version, config),
      server_(server), 
      min_compute_capability_(min_compute_capability),
      auto_complete_config_(auto_complete_config),
      backend_cmdline_config_map_(backend_cmdline_config_map),
      host_policy_map_(host_policy_map), 
      device_blocking_(false),
      localized_model_dir_(localized_model_dir), 
      backend_(backend),
      state_(nullptr) {}

  // Merges the global backend configs with the specific
  // backend configs.
  static Status ResolveBackendConfigs(const BackendCmdlineConfigMap& backend_cmdline_config_map,
                                      const std::string& backend_name, 
                                      BackendCmdlineConfig& config);

  // Sets defaults for some backend configurations when none are specified on
  // the command line.
  static Status SetBackendConfigDefaults(BackendCmdlineConfig& config);

  // Get the search paths to the backend shared library.
  static std::vector<std::string> GetBackendLibrarySearchPaths(const std::string& model_path, 
                                                               int64_t version,
                                                               const std::string& backend_dir, 
                                                               const std::string& backend_name);

  // Get backend library directory and path, and search paths for the library
  // and whether the backend is based on Python backend. The model configuration
  // runtime field will be updated if left empty.
  static Status GetBackendLibraryProperties(const std::string& model_path, 
                                            int64_t version,
                                            const std::string& backend_dir, 
                                            const std::string& backend_name,
                                            inference::ModelConfig* model_config, 
                                            bool* is_python_based_backend,
                                            std::vector<std::string>* search_paths, 
                                            std::string* backend_libdir,
                                            std::string* backend_libpath);

  // Get 'backend_libname', 'backend_libdir', 'backend_libpath' and
  // 'is_python_based_backend' by searching for different possible backend
  // library names on 'search_paths'. Searching for Python based backend
  // runtime is limited to 'backend_dir'.
  static Status GetBackendRuntimeLibraryName(const std::string& backend_dir, 
                                             const std::string& backend_name,
                                             const std::vector<std::string>& search_paths,
                                             std::string* backend_libname, 
                                             std::string* backend_libdir,
                                             std::string* backend_libpath, 
                                             bool* is_python_based_backend);

  // Search for 'backend_libname' on 'search_paths'. If found, the matching
  // search path will be stored in 'backend_libdir' and the backend library path
  // will be stored in 'backend_libpath'. If not found, 'backend_libpath' will
  // be set to empty.
  static Status FindBackendLibraryPath(const std::vector<std::string>& search_paths,
                                       const std::string& backend_libname, 
                                       std::string* backend_libdir,
                                       std::string* backend_libpath);

  // Assemble the C++ runtime library name.
  static std::string AssembleCPPRuntimeLibraryName(const std::string& backend_name);

  // Backend used by this model.
  std::shared_ptr<Backend> backend_;
  // Opaque state associated with this model.
  void* state_;
  // The localized repo directory holding the model. If localization
  // required creation of a temporary local copy then that copy will
  // persist as along as this object is retained by this model.
  std::shared_ptr<LocalizedPath> localized_model_dir_;
  // The server object that owns this model. The model holds this as a
  // raw pointer because the lifetime of the server is guaranteed to
  // be longer than the lifetime of a model owned by the server.
  InferenceServer* server_;
  // The minimum supported compute capability on device.
  const double min_compute_capability_;
  // Whether the backend should attempt to auto-complete the model config.
  const bool auto_complete_config_;
  // The backend cmdline config.
  const BackendCmdlineConfigMap backend_cmdline_config_map_;
  // The host policy map.
  const HostPolicyCmdlineConfigMap host_policy_map_;
  // The device blocking. It should not be changed after the model is created.
  bool device_blocking_;
};

}