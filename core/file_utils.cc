#include "file_utils.h"
#include <fstream>

namespace core {

std::string JoinPath(std::initializer_list<std::string> segments) {
  std::string joined;
  for (const auto& seg : segments) {
    if (joined.empty()) {
      joined = seg;
    } else if (IsAbsolutePath(seg)) {
      if (joined[joined.size() - 1] == '/') {
        joined.append(seg.substr(1));
      } else {
        joined.append(seg);
      }
    } else {  // !IsAbsolutePath(seg)
      if (joined[joined.size() - 1] != '/') {
        joined.append("/");
      }
      joined.append(seg);
    }
  }
  return joined;
}

bool IsAbsolutePath(const std::string& path) {
  return !path.empty() && (path[0] == '/');
}

std::string BaseName(const std::string& path) {
  if (path.empty()) {
    return path;
  }
  size_t last = path.size() - 1;
  while ((last > 0) && (path[last] == '/')) {
    last -= 1;
  }
  if (path[last] == '/') {
    return std::string();
  }
  const size_t idx = path.find_last_of("/", last);
  if (idx == std::string::npos) {
    return path.substr(0, last + 1);
  }
  return path.substr(idx + 1, last - idx);
}

std::string DirName(const std::string& path) {
  if (path.empty()) {
    return path;
  }
  size_t last = path.size() - 1;
  while ((last > 0) && (path[last] == '/')) {
    last -= 1;
  }
  if (path[last] == '/') {
    return std::string("/");
  }
  const size_t idx = path.find_last_of("/", last);
  if (idx == std::string::npos) {
    return std::string(".");
  }
  if (idx == 0) {
    return std::string("/");
  }
  return path.substr(0, idx);
}

Status ReadTextFile(const std::string& path, std::string* contents) {
  std::ifstream in(path, std::ios::in | std::ios::binary);
  if (!in) {
    auto msg = "failed to open text file for read " +
               path +
              ": " +
              strerror(errno);
    return Status(Status::Code::INTERNAL, msg);
  }
  in.seekg(0, std::ios::end);
  contents->resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&(*contents)[0], contents->size());
  in.close();
  return Status::Success;
}

} // namespace core