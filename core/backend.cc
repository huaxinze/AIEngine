#include "backend.h"

#include "message.h"
#include "shared_library.h"

namespace core {

Status Backend::Create(const std::string& name,
                       const std::string& dir, 
                       const std::string& libpath,
                       const BackendCmdlineConfig& backend_cmdline_config,
                       std::shared_ptr<Backend>* backend) {
  // Create the JSON representation of the backend configuration.
  using namespace common;
  Json::Value config_json(Json::ValueType::OBJECT);
  if (!backend_cmdline_config.empty()) {
    Json::Value cmdline_json(config_json, Json::ValueType::OBJECT);
    for (const auto& pr : backend_cmdline_config) {
      RETURN_IF_ERROR(cmdline_json.AddString(pr.first.c_str(), pr.second));
    }
    RETURN_IF_ERROR(config_json.Add("cmdline", std::move(cmdline_json)));
  }
  Message backend_config(config_json);
  auto local_backend = std::shared_ptr<Backend>(new Backend(name, dir, libpath, backend_config));
  // Load the library and initialize all the entrypoints
  RETURN_IF_ERROR(local_backend->LoadBackendLibrary());
  // Backend initialization is optional... The BACKEND_Backend
  // object is this TritonBackend object. We must set set shared
  // library path to point to the backend directory in case the
  // backend library attempts to load additional shared libraries.
  if (local_backend->backend_init_fn_ != nullptr) {
    std::unique_ptr<SharedLibrary> slib;
    RETURN_IF_ERROR(SharedLibrary::Acquire(&slib));
    RETURN_IF_ERROR(slib->SetLibraryDirectory(local_backend->dir_));
    SERVER_Error* err = local_backend->backend_init_fn_(reinterpret_cast<BACKEND_Backend*>(local_backend.get()));
    RETURN_IF_ERROR(slib->ResetLibraryDirectory());
    RETURN_IF_SERVER_ERROR(err);
  }
  local_backend->UpdateAttributes();
  *backend = std::move(local_backend);
  return Status::Success;
}

Backend::Backend(const std::string& name, 
                 const std::string& dir, 
                 const std::string& libpath,
                 const Message& backend_config)
  : name_(name), dir_(dir), libpath_(libpath),  backend_config_(backend_config), state_(nullptr) {
  ClearHandles();
}

Backend::~Backend() {
  // Backend finalization is optional... The BACKEND_Backend
  // object is this Backend object.
  if (backend_fini_fn_ != nullptr) {
    backend_fini_fn_(reinterpret_cast<BACKEND_Backend*>(this));
  }
  ClearHandles();
}

void Backend::ClearHandles() {
  dlhandle_ = nullptr;
  inst_init_fn_ = nullptr;
  inst_fini_fn_ = nullptr;
  inst_exec_fn_ = nullptr;
  model_init_fn_ = nullptr;
  model_fini_fn_ = nullptr;
  backend_init_fn_ = nullptr;
  backend_fini_fn_ = nullptr;
  backend_attri_fn_ = nullptr;
}

Status Backend::LoadBackendLibrary() {
  BackendInitFn_t bifn = nullptr;
  BackendFiniFn_t bffn = nullptr;
  BackendAttriFn_t bafn = nullptr;
  ModelInitFn_t mifn = nullptr;
  ModelFiniFn_t mffn = nullptr;
  ModelInstanceInitFn_t iifn = nullptr;
  ModelInstanceFiniFn_t iffn = nullptr;
  ModelInstanceExecFn_t iefn = nullptr;
  std::unique_ptr<SharedLibrary> slib;
  RETURN_IF_ERROR(SharedLibrary::Acquire(&slib));
  RETURN_IF_ERROR(slib->OpenLibraryHandle(libpath_, &dlhandle_));
  // Backend initialize and finalize functions, optional
  RETURN_IF_ERROR(slib->GetEntrypoint(
    dlhandle_, 
    "BACKEND_Initialize",
    true /* optional */,
    reinterpret_cast<void**>(&bifn)
  ));
  RETURN_IF_ERROR(slib->GetEntrypoint(
    dlhandle_, 
    "BACKEND_Finalize",
    true /* optional */,
    reinterpret_cast<void**>(&bffn)
  ));
  // Backend attribute function, optional
  RETURN_IF_ERROR(slib->GetEntrypoint(
    dlhandle_, 
    "BACKEND_GetBackendAttribute",
    true /* optional */,
    reinterpret_cast<void**>(&bafn)
  ));
  // Model initialize and finalize functions, optional
  RETURN_IF_ERROR(slib->GetEntrypoint(
    dlhandle_, 
    "BACKEND_ModelInitialize", 
    true /* optional */,
    reinterpret_cast<void**>(&mifn)
  ));
  RETURN_IF_ERROR(slib->GetEntrypoint(
    dlhandle_, 
    "BACKEND_ModelFinalize", 
    true /* optional */,
    reinterpret_cast<void**>(&mffn)
  ));
  // Model instance initialize and finalize functions, optional
  RETURN_IF_ERROR(slib->GetEntrypoint(
    dlhandle_, 
    "BACKEND_ModelInstanceInitialize", 
    true /* optional */,
    reinterpret_cast<void**>(&iifn)
  ));
  RETURN_IF_ERROR(slib->GetEntrypoint(
    dlhandle_, 
    "BACKEND_ModelInstanceFinalize", 
    true /* optional */,
    reinterpret_cast<void**>(&iffn)
  ));

  // Model instance execute function, required
  RETURN_IF_ERROR(slib->GetEntrypoint(
    dlhandle_,
    "BACKEND_ModelInstanceExecute", 
    false /* optional */,
    reinterpret_cast<void**>(&iefn)
  ));
  inst_init_fn_ = iifn;
  inst_fini_fn_ = iffn;
  inst_exec_fn_ = iefn;
  model_init_fn_ = mifn;
  model_fini_fn_ = mffn;
  backend_init_fn_ = bifn;
  backend_fini_fn_ = bffn;
  backend_attri_fn_ = bafn;

  return Status::Success;
}

Status Backend::UpdateAttributes() {
  if (backend_attri_fn_ == nullptr) {
    return Status::Success;
  }
  // Create an Attribute object for the backend to fill, note that it copies
  // some fields from 'attributes_' while the others use default value. This
  // is an ad hoc way to determine whether the attribute is set by the backend
  // and keep / update current value.
  Attribute latest;
  latest.exec_policy_ = attributes_.exec_policy_;
  RETURN_IF_SERVER_ERROR(backend_attri_fn_(reinterpret_cast<BACKEND_Backend*>(this), reinterpret_cast<BACKEND_BackendAttribute*>(&latest)));
  // Update attributes that were set
  attributes_.exec_policy_ = latest.exec_policy_;
  if (!latest.preferred_groups_.empty()) {
    attributes_.preferred_groups_ = latest.preferred_groups_;
  }
  attributes_.parallel_instance_loading_ = latest.parallel_instance_loading_;
  return Status::Success;
}

} // namespace core