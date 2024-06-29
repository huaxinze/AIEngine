#include "model_config_utils.h"

#include "file_utils.h"

namespace core {

Status GetModelVersionFromPath(const std::string& path, 
                               int64_t* version) {
  auto version_dir = BaseName(path);
  try {
    *version = std::atoll(version_dir.c_str());
  } catch (...) {
    return Status(
      Status::Code::INTERNAL,
      "unable to determine model version from " + path
    );
  }
  return Status::Success;
}

} // namespace core