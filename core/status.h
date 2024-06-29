#pragma once

#include <string>#pragma once

#include <string>
#include "common/error.h"

class Status : public common::Error {
 public:
  // Construct a status from a code with no message.
  explicit Status(Code code = Code::SUCCESS)
    : common::Error(code) {
  }

  // Construct a status from a code and message.
  Status(Code code, const std::string& msg) 
    : common::Error(code, msg) {
  }

  // Construct a status from a code and message.
  explicit Status(const common::Error& error) 
    : common::Error(error) {
  }

  // Convenience "success" value. Can be used as Error::Success to
  // indicate no error.
  static const Status Success;

  // Return the code for this status.
  Code StatusCode() const { return code_; }
};

#define RETURN_IF_ERROR(S)        \
  do {                            \
    const Status& status__ = (S); \
    if (!status__.IsOk()) {       \
      return status__;            \
    }                             \
  } while (false)

const Status Status::Success(Status::Code::SUCCESS);

Status CommonErrorToStatus(const common::Error& error) {
  return Status(error);
}
