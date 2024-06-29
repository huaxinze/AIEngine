#pragma once

#include <string>
#include "status.h"

namespace core {

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

/// read string content from a file.
/// \param path The file path.
/// \return the content of file.
Status ReadTextFile(const std::string& path, std::string* contents);

} // namespace core