#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _COMPILING_SERVER
#if defined(_MSC_VER)
#define SERVER_DECLSPEC __declspec(dllexport)
#define SERVER_ISPEC __declspec(dllimport)
#elif defined(__GNUC__)
#define SERVER_DECLSPEC __attribute__((__visibility__("default")))
#define SERVER_ISPEC
#else
#define SERVER_DECLSPEC
#define SERVER_ISPEC
#endif
#else
#if defined(_MSC_VER)
#define SERVER_DECLSPEC __declspec(dllimport)
#define SERVER_ISPEC __declspec(dllexport)
#else
#define SERVER_DECLSPEC
#define SERVER_ISPEC
#endif
#endif

struct SERVER_BufferAttributes;
struct SERVER_Error;
struct SERVER_InferenceRequest;
struct SERVER_InferenceResponse;
struct SERVER_InferenceTrace;
struct SERVER_Message;
struct SERVER_Metrics;
struct SERVER_Parameter;
struct SERVER_ResponseAllocator;
struct SERVER_Server;
struct SERVER_ServerOptions;
struct SERVER_Metric;
struct SERVER_MetricFamily;

typedef enum SERVER_errorcode_enum {
  SERVER_ERROR_UNKNOWN,
  SERVER_ERROR_INTERNAL,
  SERVER_ERROR_NOT_FOUND,
  SERVER_ERROR_INVALID_ARG,
  SERVER_ERROR_UNAVAILABLE,
  SERVER_ERROR_UNSUPPORTED,
  SERVER_ERROR_ALREADY_EXISTS,
  SERVER_ERROR_CANCELLED
} SERVER_Error_Code;

/// Create a new error object. The caller takes ownership of the
/// SERVER_Error object and must call SERVER_ErrorDelete to
/// release the object.
///
/// \param code The error code.
/// \param msg The error message.
/// \return A new SERVER_Error object.
SERVER_DECLSPEC 
struct SERVER_Error* SERVER_ErrorNew(SERVER_Error_Code code, const char* msg);

/// Delete an error object.
///
/// \param error The error object.
SERVER_DECLSPEC 
void SERVER_ErrorDelete(struct SERVER_Error* error);

/// Get the error code.
///
/// \param error The error object.
/// \return The error code.
SERVER_DECLSPEC 
SERVER_Error_Code SERVER_ErrorCode(struct SERVER_Error* error);

/// Get the string representation of an error code. The returned
/// string is not owned by the caller and so should not be modified or
/// freed. The lifetime of the returned string extends only as long as
/// 'error' and must not be accessed once 'error' is deleted.
///
/// \param error The error object.
/// \return The string representation of the error code.
SERVER_DECLSPEC 
const char* SERVER_ErrorCodeString(struct SERVER_Error* error);

/// Get the error message. The returned string is not owned by the
/// caller and so should not be modified or freed. The lifetime of the
/// returned string extends only as long as 'error' and must not be
/// accessed once 'error' is deleted.
///
/// \param error The error object.
/// \return The error message.
SERVER_DECLSPEC 
const char* SERVER_ErrorMessage(struct SERVER_Error* error);

#ifdef __cplusplus
}
#endif