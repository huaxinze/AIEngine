#include "status.h"

namespace core {

const Status Status::Success(Status::Code::SUCCESS);

Status CommonErrorToStatus(const common::Error& error) {
  return Status(error);
}

Status::Code ServerErrorCodeToStatusCode(SERVER_Error_Code code) {
  switch (code) {
    case SERVER_ERROR_UNKNOWN:
      return Status::Code::UNKNOWN;
    case SERVER_ERROR_INTERNAL:
      return Status::Code::INTERNAL;
    case SERVER_ERROR_NOT_FOUND:
      return Status::Code::NOT_FOUND;
    case SERVER_ERROR_INVALID_ARG:
      return Status::Code::INVALID_ARG;
    case SERVER_ERROR_UNAVAILABLE:
      return Status::Code::UNAVAILABLE;
    case SERVER_ERROR_UNSUPPORTED:
      return Status::Code::UNSUPPORTED;
    case SERVER_ERROR_ALREADY_EXISTS:
      return Status::Code::ALREADY_EXISTS;
    case SERVER_ERROR_CANCELLED:
      return Status::Code::CANCELLED;
    default:
      break;
  }
  return Status::Code::UNKNOWN;
}

SERVER_Error_Code StatusCodeToServerErrorCode(Status::Code status_code) {
  switch (status_code) {
    case Status::Code::UNKNOWN:
      return SERVER_ERROR_UNKNOWN;
    case Status::Code::INTERNAL:
      return SERVER_ERROR_INTERNAL;
    case Status::Code::NOT_FOUND:
      return SERVER_ERROR_NOT_FOUND;
    case Status::Code::INVALID_ARG:
      return SERVER_ERROR_INVALID_ARG;
    case Status::Code::UNAVAILABLE:
      return SERVER_ERROR_UNAVAILABLE;
    case Status::Code::UNSUPPORTED:
      return SERVER_ERROR_UNSUPPORTED;
    case Status::Code::ALREADY_EXISTS:
      return SERVER_ERROR_ALREADY_EXISTS;
    case Status::Code::CANCELLED:
      return SERVER_ERROR_CANCELLED;
    default:
      break;
  }
  return SERVER_ERROR_UNKNOWN;
}

}