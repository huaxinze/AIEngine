#include "backend_manager.h"
#include "backend.h"
#include <mutex>

namespace core {

static std::weak_ptr<BackendManager> backend_manager_;
static std::mutex mu_;

Status BackendManager::Create(std::shared_ptr<BackendManager>* manager) {
  std::lock_guard<std::mutex> lock(mu_);
  *manager = backend_manager_.lock();
  if (*manager != nullptr) {
    return Status::Success;
  }
  manager->reset(new BackendManager());
  backend_manager_ = *manager;
  return Status::Success;
}

Status BackendManager::CreateBackend(const std::string& name, 
                                     const std::string& dir, 
                                     const std::string& libpath,
                                     const BackendCmdlineConfig& backend_cmdline_config,
                                     std::shared_ptr<Backend>* backend) {
  std::lock_guard<std::mutex> lock(mu_);
  const auto& itr = backend_map_.find(libpath);
  // If backend already exists, re-use it.
  if (itr != backend_map_.end()) {
    *backend = itr->second;
    if ((*backend)->Name() == name) {
      return Status::Success;
    }
  }
  RETURN_IF_ERROR(Backend::Create(name, dir, libpath, backend_cmdline_config, backend));
  backend_map_.insert({libpath, *backend});
  return Status::Success;
}

Status BackendManager::BackendState(std::unique_ptr<BackendStateMap>* backend_state) {
  std::lock_guard<std::mutex> lock(mu_);
  std::unique_ptr<BackendStateMap> backend_state_map(new BackendStateMap);
  for (const auto& backend_pair : backend_map_) {
    auto& libpath = backend_pair.first;
    auto backend = backend_pair.second;
    const char* backend_config;
    size_t backend_config_size;
    backend->BackendConfig().Serialize(&backend_config, &backend_config_size);
    backend_state_map->insert({backend->Name(), std::vector<std::string>{libpath, backend_config}});
  }
  *backend_state = std::move(backend_state_map);
  return Status::Success;
}


} // namespace core