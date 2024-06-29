#pragma once

#include <stdint.h>

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

};