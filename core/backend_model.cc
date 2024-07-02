#include "backend_model.h"
#include "backend_config.h"
#include "server.h"
#include "shared_library.h"

namespace core {

Status BackendModel::Create(InferenceServer* server, 
                            const std::string& model_path,
                            const BackendCmdlineConfigMap& backend_cmdline_config_map,
                            const HostPolicyCmdlineConfigMap& host_policy_map,
                            const int64_t version, 
                            inference::ModelConfig model_config,
                            const bool is_config_provided, 
                            std::unique_ptr<BackendModel>* model) {
  model->reset();
  // The model configuration must specify a backend.
  const std::string& backend_name = model_config.backend();
  if (backend_name.empty()) {
    auto msg = "must specify 'backend' for '" + model_config.name() + "'";
    return Status(Status::Code::INVALID_ARG, msg);
  }
  // Localize the content of the model repository corresponding to
  // 'model_path'. This model holds a handle to the localized content
  // so that it persists as long as the model is loaded.
  auto localized_model_dir = std::make_shared<LocalizedPath>(model_path);
  // Get some internal configuration values needed for initialization.
  std::string backend_dir;
  RETURN_IF_ERROR(BackendConfigurationGlobalBackendsDirectory(
      backend_cmdline_config_map, &backend_dir));

  bool auto_complete_config = false;
  RETURN_IF_ERROR(BackendConfigurationAutoCompleteConfig(
      backend_cmdline_config_map, &auto_complete_config));

  double min_compute_capability = 0;
  RETURN_IF_ERROR(BackendConfigurationMinComputeCapability(
      backend_cmdline_config_map, &min_compute_capability));

  std::string specialized_backend_name;
  RETURN_IF_ERROR(BackendConfigurationSpecializeBackendName(
      backend_cmdline_config_map, backend_name, &specialized_backend_name));

  bool is_python_based_backend = false;
  std::vector<std::string> search_paths = GetBackendLibrarySearchPaths(
      model_path, version, backend_dir, backend_name);
  std::string backend_libdir, backend_libpath;
  RETURN_IF_ERROR(GetBackendLibraryProperties(
      localized_model_dir->Path(), version, backend_dir,
      specialized_backend_name, &model_config, &is_python_based_backend,
      &search_paths, &backend_libdir, &backend_libpath));
  // Resolve the global backend configuration with the specific backend
  // configuration
  BackendCmdlineConfig config;
  RETURN_IF_ERROR(ResolveBackendConfigs(
      backend_cmdline_config_map,
      (is_python_based_backend ? kPythonBackend : backend_name), config));

  RETURN_IF_ERROR(SetBackendConfigDefaults(config));
  std::shared_ptr<Backend> backend;
  RETURN_IF_ERROR(server->GetBackendManager()->CreateBackend(
      backend_name, backend_libdir, backend_libpath, config, &backend));
  // Normalize backend-dependent config
  {
    const auto& attributes = backend->BackendAttributes();
    // [WIP] formalize config normalization / validation
    RETURN_IF_ERROR(NormalizeInstanceGroup(
        min_compute_capability, attributes.preferred_groups_, &model_config));
    RETURN_IF_ERROR(
        ValidateInstanceGroup(model_config, min_compute_capability));
  }
  // Create and initialize the model.
  std::unique_ptr<BackendModel> local_model(new BackendModel(
      server, localized_model_dir, backend, min_compute_capability, version,
      model_config, auto_complete_config, backend_cmdline_config_map,
      host_policy_map));

  BackendModel* raw_local_model = local_model.get();
  // Model initialization is optional... The TRITONBACKEND_Model object is this
  // TritonModel object.
  if (backend->ModelInitFn() != nullptr) {
    // We must set set shared library path to point to the backend directory in
    // case the backend library attempts to load additional shared libraries.
    // Currently, the set and reset function is effective only on Windows, so
    // there is no need to set path on non-Windows.
    // However, parallel model loading will not see any speedup on Windows and
    // the global lock inside the SharedLibrary is a WAR.
    // [FIXME] Reduce lock WAR on SharedLibrary (DLIS-4300)
#ifdef _WIN32
    std::unique_ptr<SharedLibrary> slib;
    RETURN_IF_ERROR(SharedLibrary::Acquire(&slib));
    RETURN_IF_ERROR(slib->SetLibraryDirectory(backend->Directory()));
#endif
    SERVER_Error* err = backend->ModelInitFn()(
      reinterpret_cast<BACKEND_Model*>(raw_local_model));
#ifdef _WIN32
    RETURN_IF_ERROR(slib->ResetLibraryDirectory());
#endif
    RETURN_IF_SERVER_ERROR(err);
  }
  // Initialize the model for Triton core usage
  RETURN_IF_ERROR(local_model->Init(is_config_provided));
  RETURN_IF_ERROR(local_model->GetExecutionPolicy(model_config));
  // Create or update the model instances for this model.
  std::vector<std::shared_ptr<BackendModelInstance>> added_instances, 
    removed_instances;
  RETURN_IF_ERROR(local_model->PrepareInstances(
      model_config, &added_instances, &removed_instances));
  RETURN_IF_ERROR(local_model->SetConfiguredScheduler(added_instances));
  local_model->CommitInstances();
  *model = std::move(local_model);
  return Status::Success;
}

// Prepare the next set of instances on the background. Returns the instances
// that will be added and removed if the next set of instances is to be
// committed.
Status BackendModel::PrepareInstances(const inference::ModelConfig& model_config,
  std::vector<std::shared_ptr<BackendModelInstance>>* added_instances,
  std::vector<std::shared_ptr<BackendModelInstance>>* removed_instances) {
  return Status::Success;
}

Status BackendModel::SetConfiguredScheduler(
  const std::vector<std::shared_ptr<BackendModelInstance>>& new_instances) {
  return Status::Success;
}

// Replace the foreground instances with background instances.
void BackendModel::CommitInstances() {
  instances_.swap(bg_instances_);
  passive_instances_.swap(bg_passive_instances_);
  ClearBackgroundInstances();
}

void BackendModel::ClearBackgroundInstances() {
  bg_instances_.clear();
  bg_passive_instances_.clear();
}

// Gets the execution policy setting from the backend.
Status BackendModel::GetExecutionPolicy(const inference::ModelConfig& model_config) {
  // Set 'device_blocking_'
  device_blocking_ = false;
  if (backend_->ExecutionPolicy() == BACKEND_EXECUTION_DEVICE_BLOCKING) {
    device_blocking_ = true;
  }
  return Status::Success;
}

// Merges the global backend configs with the specific
// backend configs.
Status BackendModel::ResolveBackendConfigs(const BackendCmdlineConfigMap& backend_cmdline_config_map,
                                           const std::string& backend_name, 
                                           BackendCmdlineConfig& config) {
  const auto& global_itr = backend_cmdline_config_map.find(std::string());
  const auto& specific_itr = backend_cmdline_config_map.find(backend_name);
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
  for (auto& final_setting : lconfig) { config.emplace_back(final_setting); }
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