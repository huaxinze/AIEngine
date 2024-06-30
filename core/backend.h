#pragma once

#include <vector>
#include "status.h"
#include "message.h"
#include "constants.h"
#include "model_config.h"
#include "interface/IBackend.h"

namespace core {

class Backend {
 public:
  struct Attribute {
    std::vector<inference::ModelInstanceGroup> preferred_groups_;
    BACKEND_ExecutionPolicy exec_policy_{BACKEND_EXECUTION_BLOCKING};
    // Whether the backend supports loading model instances in parallel
    bool parallel_instance_loading_{false};
  };

  typedef SERVER_Error* (*ModelInitFn_t)(BACKEND_Model* model);
  typedef SERVER_Error* (*ModelFiniFn_t)(BACKEND_Model* model);
  typedef SERVER_Error* (*ModelInstanceInitFn_t)(BACKEND_ModelInstance* instance);
  typedef SERVER_Error* (*ModelInstanceFiniFn_t)(BACKEND_ModelInstance* instance);
  typedef SERVER_Error* (*ModelInstanceExecFn_t)(BACKEND_ModelInstance* instance, BACKEND_Request** requests, const uint32_t request_cnt);

  static Status Create(const std::string& name, 
                       const std::string& dir,
                       const std::string& libpath,
                       const BackendCmdlineConfig& backend_cmdline_config,
                       std::shared_ptr<Backend>* backend);
  ~Backend();

  void* State() { return state_; }
  void SetState(void* state) { state_ = state; }
  const std::string& Name() const { return name_; }
  const std::string& Directory() const { return dir_; }
  const std::string& LibPath() const { return libpath_; }
  const Message& BackendConfig() const { return backend_config_; }
  const Attribute& BackendAttributes() const { return attributes_; }
  BACKEND_ExecutionPolicy ExecutionPolicy() const { return attributes_.exec_policy_; }
  void SetExecutionPolicy(const BACKEND_ExecutionPolicy policy) { attributes_.exec_policy_ = policy; }

  ModelInitFn_t ModelInitFn() const { return model_init_fn_; }
  ModelFiniFn_t ModelFiniFn() const { return model_fini_fn_; }
  ModelInstanceInitFn_t ModelInstanceInitFn() const { return inst_init_fn_; }
  ModelInstanceFiniFn_t ModelInstanceFiniFn() const { return inst_fini_fn_; }
  ModelInstanceExecFn_t ModelInstanceExecFn() const { return inst_exec_fn_; }

 private:
  typedef SERVER_Error* (*BackendInitFn_t)(BACKEND_Backend* backend);
  typedef SERVER_Error* (*BackendFiniFn_t)(BACKEND_Backend* backend);
  typedef SERVER_Error* (*BackendAttriFn_t)(BACKEND_Backend* backend, BACKEND_BackendAttribute* backend_attributes);

  Backend(const std::string& name, 
          const std::string& dir,
          const std::string& libpath, 
          const Message& backend_config);

  void ClearHandles();
  Status LoadBackendLibrary();
  Status UpdateAttributes();

  // Opaque state associated with the backend.
  void* state_;
  // Backend configuration as JSON
  Message backend_config_;
  // The name of the backend.
  const std::string name_;
  // Full path to the backend shared library.
  const std::string libpath_;
  // Full path to the directory holding backend shared library and
  // other artifacts.
  const std::string dir_;

  // backend attributes
  Attribute attributes_;

  // dlopen / dlsym handles
  void* dlhandle_;
  BackendInitFn_t backend_init_fn_;
  BackendFiniFn_t backend_fini_fn_;
  BackendAttriFn_t backend_attri_fn_;
  ModelInitFn_t model_init_fn_;
  ModelFiniFn_t model_fini_fn_;
  ModelInstanceInitFn_t inst_init_fn_;
  ModelInstanceFiniFn_t inst_fini_fn_;
  ModelInstanceExecFn_t inst_exec_fn_;
};

} // namespace core