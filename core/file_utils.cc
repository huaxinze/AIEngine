#include "file_utils.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#include <filesystem>

#ifdef _WIN32
// suppress the min and max definitions in Windef.h.
#define NOMINMAX
#include <Windows.h>

// _CRT_INTERNAL_NONSTDC_NAMES 1 before including Microsoft provided C Runtime
// library to expose declarations without "_" prefix to match POSIX style.
#define _CRT_INTERNAL_NONSTDC_NAMES 1
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#ifdef _WIN32
// <sys/stat.h> in Windows doesn't define S_ISDIR macro
#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#endif
#define F_OK 0
#endif

namespace core {

LocalizedPath::~LocalizedPath() {
  if (!local_path_.empty()) {
    bool is_dir = true;
    IsDirectory(local_path_, &is_dir);
    // TODO
    // DeletePath(is_dir ? local_path_ : DirName(local_path_));
  }
}

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

Status FileExists(const std::string& path, bool* exists) {
  *exists = (access(path.c_str(), F_OK) == 0);
  return Status::Success;
}

Status IsDirectory(const std::string& path, bool* is_dir) {
  *is_dir = false;
  struct stat st;
  if (stat(path.c_str(), &st) != 0) {
    auto msg = "failed to stat file " + path;
    return Status(Status::Code::INTERNAL, msg);
  }
  *is_dir = S_ISDIR(st.st_mode);
  return Status::Success;
}

bool IsChildPathEscapingParentPath(const std::string& child_path, 
                                   const std::string& parent_path) {
  // TODO
  const std::string absolute_child_path = child_path;
    // std::filesystem::weakly_canonical(child_path).string();
  const std::string absolute_parent_path = parent_path;
    // std::filesystem::canonical(parent_path).string();
  // Can use starts_with() over rfind() in C++20.
  bool is_escape = 
    absolute_child_path.rfind(absolute_parent_path, 0) != 0;
  return is_escape;
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