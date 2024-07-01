#pragma once

#include <string>
#include <vector>
#include <memory>

#include "status.h"

namespace core {

// This class stores the paths of local temporary 
// files needed for loading models from Cloud repositories
// and performs necessary cleanup after the models are loaded.
class LocalizedPath {
 public:
  // Create an object for a path that is already local.
  LocalizedPath(const std::string& original_path)
    : original_path_(original_path) {}

  // Create an object for a remote path. Store both 
  // the original path and the temporary local path.
  LocalizedPath(const std::string& original_path, 
                const std::string& local_path)
    : original_path_(original_path), local_path_(local_path) {}

  // Destructor. Remove temporary local storage 
  // associated with the object. If the local path is a
  // directory, delete the directory. If the local path 
  // is a file, delete the directory containing the file.
  ~LocalizedPath();

  // Return the localized path represented by this object.
  const std::string& Path() const {
    return (local_path_.empty()) ? original_path_ : local_path_;
  }

  // Maintain a vector of LocalizedPath that should be
  // kept available in the tmp directory for the lifetime
  // of this object
  // FIXME: Remove when no longer required
  std::vector<std::shared_ptr<LocalizedPath>> other_localized_path;

 private:
  std::string original_path_;
  std::string local_path_;
};

/// Is a path an absolute path?
/// \param path The path.
/// \return true if absolute path, false if relative path.
bool IsAbsolutePath(const std::string& path);

/// Join path segments into a longer path
/// \param segments The path segments.
/// \return the path formed by joining the segments.
std::string JoinPath(std::initializer_list<std::string> segments);

/// Get the basename of a path.
/// \param path The path.
/// \return the last segment of the path.
std::string BaseName(const std::string& path);

/// Get the dirname of a path.
/// \param path The path.
/// \return all but the last segment of the path.
std::string DirName(const std::string& path);

/// check the file is existed or not.
/// \param path The file path.
/// \param exists whether existed.
/// \return Status.
Status FileExists(const std::string& path, bool* exists);

/// check the path is a dir or not.
/// \param path The file path.
/// \param is_dir is a dir or not.
/// \return Status.
Status IsDirectory(const std::string& path, bool* is_dir);

/// check the child path escaping the parent path or not.
/// \param child_path The child path.
/// \param parent_path The parent path.
/// \return Status.
bool IsChildPathEscapingParentPath(const std::string& child_path, 
                                   const std::string& parent_path);

/// read string content from a file.
/// \param path The file path.
/// \param contents the content of file.
/// \return Status.
Status ReadTextFile(const std::string& path, std::string* contents);

} // namespace core