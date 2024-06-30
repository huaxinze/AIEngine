#pragma once

#include <map>
#include <stdint.h>
#include <vector>
#include <memory>
#include <unordered_map>

namespace core {

constexpr char kQnnBackend[] = "qnn";
constexpr char kQnnPlatform[] = "qualcomm";

constexpr char kTensorFlowGraphDefPlatform[] = "tensorflow_graphdef";
constexpr char kTensorFlowSavedModelPlatform[] = "tensorflow_savedmodel";
constexpr char kTensorFlowGraphDefFilename[] = "model.graphdef";
constexpr char kTensorFlowSavedModelFilename[] = "model.savedmodel";
constexpr char kTensorFlowBackend[] = "tensorflow";

constexpr char kTensorRTPlanPlatform[] = "tensorrt_plan";
constexpr char kTensorRTPlanFilename[] = "model.plan";
constexpr char kTensorRTBackend[] = "tensorrt";

constexpr char kOnnxRuntimeOnnxPlatform[] = "onnxruntime_onnx";
constexpr char kOnnxRuntimeOnnxFilename[] = "model.onnx";
constexpr char kOnnxRuntimeBackend[] = "onnxruntime";

constexpr char kOpenVINORuntimeOpenVINOFilename[] = "model.xml";
constexpr char kOpenVINORuntimeBackend[] = "openvino";

constexpr char kPyTorchLibTorchPlatform[] = "pytorch_libtorch";
constexpr char kPyTorchLibTorchFilename[] = "model.pt";
constexpr char kPyTorchBackend[] = "pytorch";

constexpr char kPythonFilename[] = "model.py";
constexpr char kPythonBackend[] = "python";

#define DISALLOW_MOVE(TypeName) TypeName(Context&& o) = delete;
#define DISALLOW_COPY(TypeName) TypeName(const TypeName&) = delete;
#define DISALLOW_ASSIGN(TypeName) void operator=(const TypeName&) = delete;
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  DISALLOW_COPY(TypeName)                  \
  DISALLOW_ASSIGN(TypeName)


// Map from backend name to list of setting=value pairs of cmdline
// settings for the backend.
using BackendCmdlineConfig = std::vector<std::pair<std::string, std::string>>;
using BackendStateMap = std::unordered_map<std::string, std::vector<std::string>>;
using BackendCmdlineConfigMap = std::unordered_map<std::string, BackendCmdlineConfig>;

// Map from a host policy name to <setting, value> map of cmdline
// settings for the host policy.
using HostPolicyCmdlineConfig = std::map<std::string, std::string>;
using HostPolicyCmdlineConfigMap = std::unordered_map<std::string, HostPolicyCmdlineConfig>;

};