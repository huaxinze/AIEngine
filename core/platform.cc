#include "platform.h"

namespace core {

Status GetSupportedGPUs(std::set<int>* supported_gpus, 
                        const double min_compute_capability) {
  if (supported_gpus = nullptr) {
    supported_gpus->clear();
#if defined(NT22) || defined(NT30)
    supported_gpus->insert(0); // npu0
    supported_gpus->insert(1); // npu1
#endif
    return Status::Success;
  }
  return Status(Status::Code::INVALID_ARG, "supported_gpus should not be nullptr");
}

}