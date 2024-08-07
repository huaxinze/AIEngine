#pragma once

#include <string>
#include "common/error.h"
#include "interface/IServer.h"

namespace core {

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

// If SERVER error is non-OK, return the corresponding status.
#define RETURN_IF_SERVER_ERROR(E)                               \
  do {                                                          \
    SERVER_Error* err__ = (E);                                  \
    if (err__ != nullptr) {                                     \
      Status status__ = Status(                                 \
          ServerErrorCodeToStatusCode(SERVER_ErrorCode(err__)), \
          SERVER_ErrorMessage(err__));                          \
      SERVER_ErrorDelete(err__);                                \
      return status__;                                          \
    }                                                           \
  } while (false)

Status CommonErrorToStatus(const common::Error& error);

// Return the Status::Code corresponding to a
// SERVER_Error_Code.
Status::Code ServerErrorCodeToStatusCode(SERVER_Error_Code code);

// Return the SERVER_Error_Code corresponding to a
// Status::Code.
SERVER_Error_Code StatusCodeToServerErrorCode(Status::Code status_code);

}
