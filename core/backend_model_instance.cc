#include "backend_model_instance.h"

#include <iostream>

#include "model.h"
#include "backend_model.h"

namespace core {

Status BackendModelInstance::CreateInstance(BackendModel* model, 
                                            const std::string& name, 
                                            const Signature& signature,
                                            SERVER_InstanceGroupKind kind, 
                                            int32_t device_id,
                                            const std::vector<std::string>& profile_names, 
                                            const bool passive,
                                            const std::string& host_policy_name,
                                            const inference::ModelRateLimiter& rate_limiter_config,
                                            const std::vector<SecondaryDevice>& secondary_devices,
                                            std::shared_ptr<BackendModelInstance>* backend_model_instance) {
  return Status::Success;
}

Status BackendModelInstance::Construct(BackendModel* model, 
                                       const std::string& name, 
                                       const Signature& signature,
                                       const SERVER_InstanceGroupKind kind, 
                                       const int32_t device_id,
                                       const std::vector<std::string>& profile_names, 
                                       const bool passive,
                                       const std::string& host_policy_name,
                                       const HostPolicyCmdlineConfig& host_policy,
                                       const inference::ModelRateLimiter& rate_limiter_config,
                                       const std::vector<SecondaryDevice>& secondary_devices,
                                       std::shared_ptr<BackendModelInstance>* model_instance) {
  return Status::Success;
}

Status BackendModelInstance::SetBackendThread(const SERVER_InstanceGroupKind kind, 
                                              const int32_t device_id,
                                              const bool device_blocking) {
  return Status::Success;
}

void BackendModelInstance::Execute(std::vector<BACKEND_Request*>& backend_requests) {
  
}

Status BackendModelInstance::Schedule(std::vector<std::unique_ptr<InferenceRequest>>&& requests) {
  return Status::Success;
}

BackendModelInstance::~BackendModelInstance() {

}

Status BackendModelInstance::
BackendThread::CreateBackendThread(const std::string name, 
                                   BackendModelInstance* model_instance, 
                                   const int nice,
                                   const int32_t device_id,
                                   std::unique_ptr<BackendThread>* backend_thread) {
  BackendThread* raw_backend_thread = new BackendThread(
    name, model_instance->Model(), nice, device_id);
  std::unique_ptr<BackendThread> runner(raw_backend_thread);
  runner->AddModelInstance(model_instance);
  runner->backend_thread_ = std::thread([raw_backend_thread]() {
    raw_backend_thread->BackendThreadFunc(); 
  });
  backend_thread->reset(runner.release());
  return Status::Success;
}

void BackendModelInstance::
BackendThread::StopBackendThread() {
  if (backend_thread_.joinable()) {
    // TODO: Signal the backend thread to exit and then wait for it...
    backend_thread_.join();
  }
}

void BackendModelInstance::
BackendThread::AddModelInstance(BackendModelInstance* model_instance) {
  model_instances_.push_back(model_instance);
}

Status BackendModelInstance::
BackendThread::InitAndWarmUpModelInstance(BackendModelInstance* model_instance) {
  return Status::Success;
}

void BackendModelInstance:: 
BackendThread::BackendThreadFunc() {
#ifndef _WIN32
  if (setpriority(PRIO_PROCESS, syscall(SYS_gettid), nice_) == 0) {
    std::cout << "Starting backend thread for " << name_ << " at nice "
              << nice_ << " on device " << device_id_ << "..." << std::endl;
  } else {
    std::cout << "Starting backend thread for " << name_
              << " at default nice (requested nice " << nice_ << " failed)"
              << " on device " << device_id_ << "..." << std::endl;
  }
#else
  std::cout << "Starting backend thread for " << name_
            << " at default nice on device " << device_id_ << "..." << std::endl;
#endif
  bool should_exit = false;
  while (!should_exit) {
    // std::shared_ptr<Payload> payload;
    // model_->Server()->GetRateLimiter()->DequeuePayload(
    //     model_instances_, &payload);
    // NVTX_RANGE(nvtx_, "BackendThread " + name_);
    // payload->Execute(&should_exit);
    // model_instances_.push_back(payload->GetInstance());
    // // Release the payload to the RateLimiter
    // model_->Server()->GetRateLimiter()->PayloadRelease(payload);
  }
  std::cout << "Stopping the backend thread for " << name_ << " ..." << std::endl;
}


}