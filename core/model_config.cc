#include "model_config.h"

namespace core { 

int64_t GetElementCount(const DimsList& dims) {
  bool first = true;
  int64_t cnt = 0;
  for (auto dim : dims) {
    if (dim == WILDCARD_DIM) {
      return -1;
    }
    if (first) {
      cnt = dim;
      first = false;
    } else {
      cnt *= dim;
    }
  }
  return cnt;
}

int64_t GetElementCount(const std::vector<int64_t>& dims) {
  bool first = true;
  int64_t cnt = 0;
  for (auto dim : dims) {
    if (dim == WILDCARD_DIM) {
      return -1;
    }
    if (first) {
      cnt = dim;
      first = false;
    } else {
      cnt *= dim;
    }
  }
  return cnt;
}

int64_t GetElementCount(const inference::ModelInput& mio) {
  return GetElementCount(mio.dims());
}

int64_t GetElementCount(const inference::ModelOutput& mio) {
  return GetElementCount(mio.dims());
}

bool IsFixedSizeDataType(const inference::DataType dtype) {
   return dtype != inference::DataType::TYPE_STRING;
}

size_t GetDataTypeByteSize(const inference::DataType dtype) {
  switch (dtype) {
    case inference::DataType::TYPE_BOOL:
      return 1;
    case inference::DataType::TYPE_UINT8:
      return 1;
    case inference::DataType::TYPE_UINT16:
      return 2;
    case inference::DataType::TYPE_UINT32:
      return 4;
    case inference::DataType::TYPE_UINT64:
      return 8;
    case inference::DataType::TYPE_INT8:
      return 1;
    case inference::DataType::TYPE_INT16:
      return 2;
    case inference::DataType::TYPE_INT32:
      return 4;
    case inference::DataType::TYPE_INT64:
      return 8;
    case inference::DataType::TYPE_FP16:
      return 2;
    case inference::DataType::TYPE_FP32:
      return 4;
    case inference::DataType::TYPE_FP64:
      return 8;
    case inference::DataType::TYPE_STRING:
      return 0;
    case inference::DataType::TYPE_BF16:
      return 2;
    default:
      break;
  }
  return 0;
}

int64_t GetByteSize(const inference::DataType& dtype, 
                    const DimsList& dims) {
  size_t dt_size = GetDataTypeByteSize(dtype);
  if (dt_size == 0) {
    return -1;
  }
  int64_t cnt = GetElementCount(dims);
  if (cnt == -1) {
    return -1;
  }
  return cnt * dt_size;
}

int64_t GetByteSize(const inference::DataType& dtype, 
                    const std::vector<int64_t>& dims) {
  size_t dt_size = GetDataTypeByteSize(dtype);
  if (dt_size == 0) {
    return -1;
  }
  int64_t cnt = GetElementCount(dims);
  if (cnt == -1) {
    return -1;
  }
  return cnt * dt_size;
}

int64_t GetByteSize(const int batch_size, 
                    const inference::DataType& dtype,
                    const DimsList& dims) {
  if (dims.size() == 0) {
    return batch_size * GetDataTypeByteSize(dtype);
  }
  int64_t bs = GetByteSize(dtype, dims);
  if (bs == -1) {
    return -1;
  }
  return std::max(1, batch_size) * bs;
}

int64_t GetByteSize(const int batch_size, 
                    const inference::DataType& dtype,
                    const std::vector<int64_t>& dims) {
  if (dims.size() == 0) {
    return batch_size * GetDataTypeByteSize(dtype);
  }
  int64_t bs = GetByteSize(dtype, dims);
  if (bs == -1) {
    return -1;
  }
  return std::max(1, batch_size) * bs;
}

int64_t GetByteSize(const inference::ModelInput& mio) {
  return GetByteSize(mio.data_type(), mio.dims());
}

int64_t GetByteSize(const inference::ModelOutput& mio) {
  return GetByteSize(mio.data_type(), mio.dims());
}

bool CompareDims(const DimsList& dims0,
                const DimsList& dims1) {
  if (dims0.size() != dims1.size()) {
    return false;
  }
  for (int i = 0; i < dims0.size(); ++i) {
    if (dims0[i] != dims1[i]) {
      return false;
    }
  }
  return true;
}

bool CompareDims(const std::vector<int64_t>& dims0, 
                 const std::vector<int64_t>& dims1) {
  if (dims0.size() != dims1.size()) {
    return false;
  }
  for (size_t i = 0; i < dims0.size(); ++i) {
    if (dims0[i] != dims1[i]) {
      return false;
    }
  }
  return true;
}

bool CompareDimsWithWildcard(const DimsList& dims0, 
                             const DimsList& dims1) {
  if (dims0.size() != dims1.size()) {
    return false;
  }
  for (int i = 0; i < dims0.size(); ++i) {
    if ((dims0[i] != WILDCARD_DIM) &&
        (dims1[i] != WILDCARD_DIM) &&
        (dims0[i] != dims1[i])) {
      return false;
    }
  }
  return true;
}

bool CompareDimsWithWildcard(const DimsList& dims0, 
                             const std::vector<int64_t>& dims1) {
  if (dims0.size() != (int64_t)dims1.size()) {
    return false;
  }
  for (int i = 0; i < dims0.size(); ++i) {
    if ((dims0[i] != WILDCARD_DIM) &&
        (dims1[i] != WILDCARD_DIM) &&
        (dims0[i] != dims1[i])) {
      return false;
    }
  }
  return true;
}

std::string DimsListToString(const DimsList& dims) {
  bool first = true;
  std::string str("[");
  for (const auto& dim : dims) {
    if (!first) {
      str += ",";
    }
    str += std::to_string(dim);
    first = false;
  }
  str += "]";
  return str;
}

std::string DimsListToString(const std::vector<int64_t>& dims, 
                             const int start_idx) {
  int idx = 0;
  std::string str("[");
  for (const auto& dim : dims) {
    if (idx >= start_idx) {
      if (idx > start_idx) {
        str += ",";
      }
      str += std::to_string(dim);
    }
    idx++;
  }
  str += "]";
  return str;
}

const char* DataTypeToProtocolString(const inference::DataType dtype) {
  switch (dtype) {
    case inference::DataType::TYPE_BOOL:
      return "BOOL";
    case inference::DataType::TYPE_UINT8:
      return "UINT8";
    case inference::DataType::TYPE_UINT16:
      return "UINT16";
    case inference::DataType::TYPE_UINT32:
      return "UINT32";
    case inference::DataType::TYPE_UINT64:
      return "UINT64";
    case inference::DataType::TYPE_INT8:
      return "INT8";
    case inference::DataType::TYPE_INT16:
      return "INT16";
    case inference::DataType::TYPE_INT32:
      return "INT32";
    case inference::DataType::TYPE_INT64:
      return "INT64";
    case inference::DataType::TYPE_FP16:
      return "FP16";
    case inference::DataType::TYPE_FP32:
      return "FP32";
    case inference::DataType::TYPE_FP64:
      return "FP64";
    case inference::DataType::TYPE_STRING:
      return "BYTES";
    case inference::DataType::TYPE_BF16:
      return "BF16";
    default:
      break;
  }
  return "<invalid>";
}

inference::DataType ProtocolStringToDataType(const char* dtype, size_t len) {
  if (len < 4 || len > 6) {
    return inference::DataType::TYPE_INVALID;
  }
  if ((*dtype == 'I') && (len != 6)) {
    if ((dtype[1] == 'N') && (dtype[2] == 'T')) {
      if ((dtype[3] == '8') && (len == 4)) {
        return inference::DataType::TYPE_INT8;
      } else if ((dtype[3] == '1') && (dtype[4] == '6')) {
        return inference::DataType::TYPE_INT16;
      } else if ((dtype[3] == '3') && (dtype[4] == '2')) {
        return inference::DataType::TYPE_INT32;
      } else if ((dtype[3] == '6') && (dtype[4] == '4')) {
        return inference::DataType::TYPE_INT64;
      }
    }
  } else if ((*dtype == 'U') && (len != 4)) {
    if ((dtype[1] == 'I') && (dtype[2] == 'N') && (dtype[3] == 'T')) {
      if ((dtype[4] == '8') && (len == 5)) {
        return inference::DataType::TYPE_UINT8;
      } else if ((dtype[4] == '1') && (dtype[5] == '6')) {
        return inference::DataType::TYPE_UINT16;
      } else if ((dtype[4] == '3') && (dtype[5] == '2')) {
        return inference::DataType::TYPE_UINT32;
      } else if ((dtype[4] == '6') && (dtype[5] == '4')) {
        return inference::DataType::TYPE_UINT64;
      }
    }
  } else if ((*dtype == 'F') && (dtype[1] == 'P') && (len == 4)) {
    if ((dtype[2] == '1') && (dtype[3] == '6')) {
      return inference::DataType::TYPE_FP16;
    } else if ((dtype[2] == '3') && (dtype[3] == '2')) {
      return inference::DataType::TYPE_FP32;
    } else if ((dtype[2] == '6') && (dtype[3] == '4')) {
      return inference::DataType::TYPE_FP64;
    }
  } else if (*dtype == 'B') {
    switch (dtype[1]) {
      case 'Y':
        if (!strcmp(dtype + 2, "TES")) {
          return inference::DataType::TYPE_STRING;
        }
        break;
      case 'O':
        if (!strcmp(dtype + 2, "OL")) {
          return inference::DataType::TYPE_BOOL;
        }
        break;
      case 'F':
        if (!strcmp(dtype + 2, "16")) {
          return inference::DataType::TYPE_BF16;
        }
        break;
    }
  }
  return inference::DataType::TYPE_INVALID;
}

inference::DataType ProtocolStringToDataType(const std::string& dtype) {
  return ProtocolStringToDataType(dtype.c_str(), dtype.size());
}

} // namespace core