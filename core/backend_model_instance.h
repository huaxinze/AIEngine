#pragma once

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <deque>

#include "status.h"
#include "message.h"
#include "constants.h"
#include "interface/IServer.h"
#include "interface/IBackend.h"
#include "model_config_utils.h"

namespace core {

class BackendModel;
class InferenceRequest;

class BackendModelInstance {
 public:
  struct SecondaryDevice {
    SecondaryDevice(const std::string kind, const int64_t id) : kind_(kind), id_(id) 
    {
    }
    const int64_t id_;
    const std::string kind_;
  };
  
  class Signature {
   public:
    Signature(const inference::ModelInstanceGroup& group_config, 
              int32_t device_id)
      : group_config_(group_config), 
        device_id_(device_id),
        hash_(std::hash<std::string>{}(std::to_string(device_id_) +
          InstanceConfigSignature(group_config_))) 
    {
    }
    std::size_t Hash() const { return hash_; }
    bool operator==(const Signature& rhs) const {
      bool ret = EquivalentInInstanceConfig(group_config_, rhs.group_config_);
      return device_id_ == rhs.device_id_ && ret;
    }
    bool operator!=(const Signature& rhs) const { return !(*this == rhs); }

   private:
    const std::size_t hash_;
    const int32_t device_id_;
    const inference::ModelInstanceGroup group_config_;
  };

  static Status CreateInstance(BackendModel* model, 
                               const std::string& name, 
                               const Signature& signature,
                               SERVER_InstanceGroupKind kind, 
                               int32_t device_id,
                               const std::vector<std::string>& profile_names, 
                               const bool passive,
                               const std::string& host_policy_name,
                               const inference::ModelRateLimiter& rate_limiter_config,
                               const std::vector<SecondaryDevice>& secondary_devices,
                               std::shared_ptr<BackendModelInstance>* backend_model_instance);
  ~BackendModelInstance();

  Status Schedule(std::vector<std::unique_ptr<InferenceRequest>>&& requests);

  void* State() { return state_; }
  BackendModel* Model() const { return model_; }
  bool IsPassive() const { return passive_; }
  void SetState(void* state) { state_ = state; }
  int32_t DeviceId() const { return device_id_; }
  Signature& GetSignature() { return signature_; }
  const std::string& Name() const { return name_; }
  SERVER_InstanceGroupKind Kind() const { return kind_; }
  const HostPolicyCmdlineConfig& HostPolicy() const { return host_policy_; }
  const Message& HostPolicyMessage() const { return host_policy_message_; }
  const std::vector<std::string>& Profiles() const { return profile_names_; }
  const std::vector<SecondaryDevice>& SecondaryDevices() const { return secondary_devices_; }

 private:
  class BackendThread {
   public:
    static Status CreateBackendThread(const std::string name, 
                                      BackendModelInstance* model, 
                                      const int nice,
                                      const int32_t device_id,
                                      std::unique_ptr<BackendThread>* backend_thread);
    ~BackendThread() {
      StopBackendThread();
    }
    void StopBackendThread();
    void AddModelInstance(BackendModelInstance* model_instance);
    Status InitAndWarmUpModelInstance(BackendModelInstance* model_instance);

   private:
    BackendThread(const std::string& name, 
                  BackendModel* model, 
                  const int nice,
                  const int32_t device_id)
      : name_(name), nice_(nice), device_id_(device_id), model_(model) {}
  
    void BackendThreadFunc();
    const std::string name_;
    const int nice_;
    const int32_t device_id_;
    BackendModel* model_;
    std::deque<BackendModelInstance*> model_instances_;
    std::thread backend_thread_;
    std::atomic<bool> backend_thread_exit_;
  };

  DISALLOW_COPY_AND_ASSIGN(BackendModelInstance);
  BackendModelInstance(BackendModel* model, 
                       const std::string& name, 
                       const Signature& signature,
                       const SERVER_InstanceGroupKind kind, 
                       const int32_t device_id,
                       const std::vector<std::string>& profile_names, 
                       const bool passive,
                       const HostPolicyCmdlineConfig& host_policy,
                       const Message& host_policy_message,
                       const std::vector<SecondaryDevice>& secondary_devices)
    : model_(model), name_(name), signature_(signature), kind_(kind),
      device_id_(device_id), host_policy_(host_policy),
      host_policy_message_(host_policy_message), profile_names_(profile_names),
      passive_(passive), secondary_devices_(secondary_devices), state_(nullptr)
    {}
  
  static Status Construct(BackendModel* model, 
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
                          std::shared_ptr<BackendModelInstance>* model_instance);
  Status SetBackendThread(const SERVER_InstanceGroupKind kind, 
                          const int32_t device_id,
                          const bool device_blocking);
  void Execute(std::vector<BACKEND_Request*>& backend_requests);

  // The BackendModel object that owns this instance. The instance
  // holds this as a raw pointer because the lifetime of the model is
  // guaranteed to be longer than the lifetime of an instance owned by the
  // model.
  BackendModel* model_;
  std::string name_;
  Signature signature_;
  // For CPU device_id_ is always 0. For GPU device_id_ indicates the
  // GPU device to be used by the instance.
  SERVER_InstanceGroupKind kind_;
  const int32_t device_id_;
  const HostPolicyCmdlineConfig host_policy_;
  Message host_policy_message_;
  std::vector<std::string> profile_names_;
  bool passive_;
  std::vector<SecondaryDevice> secondary_devices_;
  // Records of memory used for the model instance
  // std::map<SERVER_MemoryType, std::map<int64_t, size_t>> memory_usage_;
  mutable std::mutex usage_mtx_;
  // Opaque state associated with this model instance.
  void* state_;
  std::shared_ptr<BackendThread> backend_thread_;
};
}

namespace std {
template <>
struct hash<core::BackendModelInstance::Signature> {
  std::size_t operator()(
      const core::BackendModelInstance::Signature& s) const
  {
    return s.Hash();
  }
};
}  // namespace std