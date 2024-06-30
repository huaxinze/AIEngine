#include "interface/IServer.h"
#include "status.h"

#if defined(_MSC_VER)
#define API_DECLSPEC __declspec(dllexport)
#elif defined(__GNUC__)
#define API_DECLSPEC __attribute__((__visibility__("default")))
#else
#define API_DECLSPEC
#endif

class ServerError {
 public:
  static SERVER_Error* Create(SERVER_Error_Code code, const char* msg);
  static SERVER_Error* Create(SERVER_Error_Code code, const std::string& msg);
  static SERVER_Error* Create(const core::Status& status);
  SERVER_Error_Code Code() const { return code_; }
  const std::string& Message() const { return msg_; }

 private:
  ServerError(SERVER_Error_Code code, const char* msg)
    : code_(code), msg_(msg) 
  {}
  ServerError(SERVER_Error_Code code, const std::string& msg)
    : code_(code), msg_(msg) 
  {}

  const std::string msg_;
  SERVER_Error_Code code_;
};

SERVER_Error* ServerError::Create(SERVER_Error_Code code, 
                                  const char* msg) {
  return reinterpret_cast<SERVER_Error*>(new ServerError(code, msg));
}

SERVER_Error* ServerError::Create(SERVER_Error_Code code, 
                                  const std::string& msg) {
  return reinterpret_cast<SERVER_Error*>(new ServerError(code, msg));
}

SERVER_Error* ServerError::Create(const core::Status& status) {
  if (status.IsOk()) {
    return nullptr;
  }
  auto code = core::StatusCodeToServerErrorCode(status.StatusCode());
  return Create(code, status.Message());
}

extern "C" {

//
// SERVER_Error
//
API_DECLSPEC 
struct SERVER_Error* SERVER_ErrorNew(SERVER_Error_Code code, const char* msg) {
  return reinterpret_cast<SERVER_Error*>(
      ServerError::Create(code, msg));
}

API_DECLSPEC 
void SERVER_ErrorDelete(SERVER_Error* error) {
  ServerError* lerror = reinterpret_cast<ServerError*>(error);
  delete lerror;
}

API_DECLSPEC 
SERVER_Error_Code SERVER_ErrorCode(SERVER_Error* error) {
  ServerError* lerror = reinterpret_cast<ServerError*>(error);
  return lerror->Code();
}

API_DECLSPEC 
const char* SERVER_ErrorCodeString(SERVER_Error* error) {
  ServerError* lerror = reinterpret_cast<ServerError*>(error);
  auto code = core::ServerErrorCodeToStatusCode(lerror->Code());
  return core::Status::CodeString(code);
}

API_DECLSPEC 
const char* SERVER_ErrorMessage(SERVER_Error* error) {
  ServerError* lerror = reinterpret_cast<ServerError*>(error);
  return lerror->Message().c_str();
}

}