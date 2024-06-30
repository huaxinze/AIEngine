#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _COMPILING_BACKEND
#if defined(_MSC_VER)
#define BACKEND_DECLSPEC __declspec(dllexport)
#define BACKEND_ISPEC __declspec(dllimport)
#elif defined(__GNUC__)
#define BACKEND_DECLSPEC __attribute__((__visibility__("default")))
#define BACKEND_ISPEC
#else
#define BACKEND_DECLSPEC
#define BACKEND_ISPEC
#endif
#else
#if defined(_MSC_VER)
#define BACKEND_DECLSPEC __declspec(dllimport)
#define BACKEND_ISPEC __declspec(dllexport)
#else
#define BACKEND_DECLSPEC
#define BACKEND_ISPEC
#endif
#endif

struct BACKEND_MemoryManager;
struct BACKEND_Input;
struct BACKEND_Output;
struct BACKEND_State;
struct BACKEND_Request;
struct BACKEND_ResponseFactory;
struct BACKEND_Response;
struct BACKEND_Backend;
struct BACKEND_Model;
struct BACKEND_ModelInstance;
struct BACKEND_ModelInstanceResponseStatistics;
struct BACKEND_BackendAttribute;
struct BACKEND_Batcher;

typedef enum BACKEND_execpolicy_enum {
  BACKEND_EXECUTION_BLOCKING,
  BACKEND_EXECUTION_DEVICE_BLOCKING
} BACKEND_ExecutionPolicy;

#ifdef __cplusplus
}
#endif