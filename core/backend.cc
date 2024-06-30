#include "backend.h"

namespace core {

Status Backend::Create(const std::string& name,
                       const std::string& dir, 
                       const std::string& libpath,
                       const BackendCmdlineConfig& backend_cmdline_config,
                       std::shared_ptr<Backend>* backend) {
  return Status::Success;
}

Backend::Backend(const std::string& name, 
                 const std::string& dir, 
                 const std::string& libpath,
                 const Message& backend_config)
  : name_(name), dir_(dir), libpath_(libpath),  backend_config_(backend_config), state_(nullptr) {
}

Backend::~Backend() {
}

} // namespace core