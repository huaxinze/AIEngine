#pragma once

#include <google/protobuf/any.pb.h>
#include <stdint.h>

#include "model_config.pb.h"

namespace core { 

/// The type for a repeated dims field (used for shape).
using DimsList = ::google::protobuf::RepeatedField<::google::protobuf::int64>;

/// The value for a dimension in a shape that indicates that
/// that dimension can take on any size.
constexpr int WILDCARD_DIM = -1;

constexpr int SCHEDULER_DEFAULT_NICE = 5;

enum Platform {
  PLATFORM_UNKNOWN          = 0,
  PLATFORM_ENSEMBLE         = 1,
  PLATFORM_QUALCOMM_QNN     = 2,
  PLATFORM_ONNXRUNTIME_ONNX = 3,
  PLATFORM_PYTORCH_LIBTORCH = 4,
};

/// Get the number of elements in a shape.
/// \param dims The shape.
/// \return The number of elements, or -1 if the number of elements
/// cannot be determined because the shape contains one or more
/// wildcard dimensions.
int64_t GetElementCount(const DimsList& dims);

/// Get the number of elements in a shape.
/// \param dims The shape.
/// \return The number of elements, or -1 if the number of elements
/// cannot be determined because the shape contains one or more
/// wildcard dimensions.
int64_t GetElementCount(const std::vector<int64_t>& dims);

/// Get the number of elements in the shape of a model input.
/// \param mio The model input.
/// \return The number of elements, or -1 if the number of elements
/// cannot be determined because the shape contains one or more
/// wildcard dimensions.
int64_t GetElementCount(const inference::ModelInput& mio);

/// Get the number of elements in the shape of a model output.
/// \param mio The model output.
/// \return The number of elements, or -1 if the number of elements
/// cannot be determined because the shape contains one or more
/// wildcard dimensions.
int64_t GetElementCount(const inference::ModelOutput& mio);

/// Are values of a datatype fixed-size, or variable-sized.
/// \param dtype The data-type.
/// \return True if datatype values are fixed-sized, false if
/// variable-sized.
bool IsFixedSizeDataType(const inference::DataType dtype);

/// Get the size of objects of a given datatype in bytes.
/// \param dtype The data-type.
/// \return The size, in bytes, of objects of the datatype, or 0 if
/// size cannot be determine (for example, values of type TYPE_STRING
/// have variable length and so size cannot be determine just from the
/// type).
size_t GetDataTypeByteSize(const inference::DataType dtype);

/// Get the size, in bytes, of a tensor based on datatype and
/// shape.
/// \param dtype The data-type.
/// \param dims The shape.
/// \return The size, in bytes, of the corresponding tensor, or -1 if
/// unable to determine the size.
int64_t GetByteSize(const inference::DataType& dtype, 
                    const DimsList& dims);

/// Get the size, in bytes, of a tensor based on datatype and
/// shape.
/// \param dtype The data-type.
/// \param dims The shape.
/// \return The size, in bytes, of the corresponding tensor, or -1 if
/// unable to determine the size.
int64_t GetByteSize(const inference::DataType& dtype, 
                    const std::vector<int64_t>& dims);

/// Get the size, in bytes, of a tensor based on batch-size, datatype
/// and shape. A tensor that has empty shape [] and non-zero
/// batch-size is sized as a tensor with shape [ batch-size ].
/// \param batch_size The batch-size. May be 0 to indicate no
/// batching.
/// \param dtype The data-type.
/// \param dims The shape.
/// \return The size, in bytes, of the corresponding tensor, or -1 if
/// unable to determine the size.
int64_t GetByteSize(const int batch_size,
                    const inference::DataType& dtype,
                    const DimsList& dims);

/// Get the size, in bytes, of a tensor based on batch-size, datatype
/// and shape. A tensor that has empty shape [] and non-zero
/// batch-size is sized as a tensor with shape [ batch-size ].
/// \param batch_size The batch-size. May be 0 to indicate no
/// batching.
/// \param dtype The data-type.
/// \param dims The shape.
/// \return The size, in bytes, of the corresponding tensor, or -1 if
/// unable to determine the size.
int64_t GetByteSize(const int batch_size, 
                    const inference::DataType& dtype, 
                    const std::vector<int64_t>& dims);

/// Get the size, in bytes, of a tensor based on ModelInput.
/// \param mio The ModelInput protobuf.
/// \return The size, in bytes, of the corresponding tensor, or -1 if
/// unable to determine the size.
int64_t GetByteSize(const inference::ModelInput& mio);

/// Get the size, in bytes, of a tensor based on ModelOutput.
/// \param mio The ModelOutput protobuf.
/// \return The size, in bytes, of the corresponding tensor, or -1 if
/// unable to determine the size.
int64_t GetByteSize(const inference::ModelOutput& mio);

/// Compare two model configuration shapes for equality. Wildcard
/// dimensions (that is, dimensions with size WILDCARD_DIM) are
/// compared literally so that to be equal the two shapes must both
/// specify WILDCARD_DIM in the same dimensions.
/// \params dims0 The first shape.
/// \params dims1 The second shape.
/// \return True if the shapes are equal, false if not equal.
bool CompareDims(const DimsList& dims0,
                 const DimsList& dims1);

/// Compare two model configuration shapes for equality. Wildcard
/// dimensions (that is, dimensions with size WILDCARD_DIM) are
/// compared literally so that to be equal the two shapes must both
/// specify WILDCARD_DIM in the same dimensions.
/// \params dims0 The first shape.
/// \params dims1 The second shape.
/// \return True if the shapes are equal, false if not equal.
bool CompareDims(const std::vector<int64_t>& dims0, 
                 const std::vector<int64_t>& dims1);

/// Compare two model configuration shapes for equality. Wildcard
/// dimensions (that is, dimensions with size WILDCARD_DIM) are
/// allowed to match with any value. So, a dimension in one shape
/// specified as WILDCARD_DIM will always match the same dimension in
/// the other shape.
/// \params dims0 The first shape.
/// \params dims1 The second shape.
/// \return True if the shapes are equal, false if not equal.
bool CompareDimsWithWildcard(const DimsList& dims0, 
                             const DimsList& dims1);

/// Compare two model configuration shapes for equality. Wildcard
/// dimensions (that is, dimensions with size WILDCARD_DIM) are
/// allowed to match with any value. So, a dimension in one shape
/// specified as WILDCARD_DIM will always match the same dimension in
/// the other shape.
/// \params dims0 The first shape.
/// \params dims1 The second shape.
/// \return True if the shapes are equal, false if not equal.
bool CompareDimsWithWildcard(const DimsList& dims0, 
                             const std::vector<int64_t>& dims1);

/// Convert a DimsList to string representation.
/// \param dims The DimsList to be converted.
/// \return String representation of the DimsList in pattern
/// "[d0,d1,...,dn]"
std::string DimsListToString(const DimsList& dims);

/// Convert a vector representing a shape to string representation.
/// \param dims The vector of dimensions to be converted.
/// \return String representation of the vector in pattern
/// "[d0,d1,...,dn]"
std::string DimsListToString(const std::vector<int64_t>& dims, 
                             const int start_idx = 0);

/// Get the server protocol string representation of a datatype.
/// \param dtype The data type.
/// \return The string representation.
const char* DataTypeToProtocolString(const inference::DataType dtype);

/// Get the datatype corresponding to a server protocol string
/// representation of a datatype.
/// \param dtype string representation.
/// \return The data type.
inference::DataType ProtocolStringToDataType(const std::string& dtype);

/// Get the datatype corresponding to a server protocol string
/// representation of a datatype.
/// \param dtype Pointer to string.
/// \param len Length of the string.
/// \return The data type.
inference::DataType ProtocolStringToDataType(const char* dtype, size_t len);

} // namespace core