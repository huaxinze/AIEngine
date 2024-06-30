#pragma once

#include "status.h"
#include "constants.h"

namespace core {

class Backend;

class BackendManager {
 public:
  static Status Create(std::shared_ptr<BackendManager>* manager);
  Status CreateBackend(const std::string& name, 
                       const std::string& dir,
                       const std::string& libpath,
                       const BackendCmdlineConfig& backend_cmdline_config,
                       std::shared_ptr<Backend>* backend);

  Status BackendState(std::unique_ptr<BackendStateMap>* backend_state);

 private:
  DISALLOW_COPY_AND_ASSIGN(BackendManager);
  BackendManager() = default;
  std::unordered_map<std::string, std::shared_ptr<Backend>> backend_map_;
};

}