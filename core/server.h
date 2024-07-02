#pragma once

#include <stddef.h>
#include <stdint.h>

#include <atomic>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include <memory>

#include "status.h"
#include "constants.h"
#include "model_config.h"
#include "rate_limiter.h"
#include "backend_manager.h"

namespace core {

class Model;
class InferenceRequest;

// Readiness status for the inference server.
enum class ServerReadyState {
  // The server is in an invalid state and will likely not response
  // correctly to any requests.
  SERVER_INVALID,

  // The server is initializing.
  SERVER_INITIALIZING,

  // The server is ready and accepting requests.
  SERVER_READY,

  // The server is exiting and will not respond to requests.
  SERVER_EXITING,

  // The server did not initialize correctly.
  SERVER_FAILED_TO_INITIALIZE
};

class InferenceServer {
 public:
  // Construct an inference server.
  InferenceServer();

  ~InferenceServer() = default;

  // Initialize the server. Return true on success, false otherwise.
  Status Init();

  // Stop the server.  Return true if all models are unloaded, false
  // if exit timeout occurs. If 'force' is true attempt to stop the
  // server even if it is not in a ready state.
  Status Stop(const bool force = false);

  // Check the model repository for changes and update server state
  // based on those changes.
  Status PollModelRepository();

  // Server health
  Status IsLive(bool* live);
  Status IsReady(bool* ready);

  // Get the Backend Manager
  const std::shared_ptr<BackendManager> GetBackendManager() {
    return backend_manager_;
  }

  // Return the pointer to RateLimiter object.
  std::shared_ptr<RateLimiter> GetRateLimiter() { return rate_limiter_; }

 private:
  const std::string version_;
  std::string id_;
  std::vector<const char*> extensions_;

  BackendCmdlineConfigMap backend_cmdline_config_map_;
  HostPolicyCmdlineConfigMap host_policy_map_;

  // Current state of the inference server.
  ServerReadyState ready_state_;

  std::shared_ptr<RateLimiter> rate_limiter_;
  // std::unique_ptr<ModelRepositoryManager> model_repository_manager_;
  std::shared_ptr<BackendManager> backend_manager_;
};

}