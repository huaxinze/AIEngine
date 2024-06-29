#include "status.h"

namespace core {

const Status Status::Success(Status::Code::SUCCESS);

Status CommonErrorToStatus(const common::Error& error) {
  return Status(error);
}

}