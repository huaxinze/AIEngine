#pragma once

#include "status.h"

namespace core {

// Get version of a model from the path containing the model
/// definition file.
/// \param path The path to the model definition file.
/// \param version Returns the version.
/// \return The error status.
Status GetModelVersionFromPath(const std::string& path, 
                               int64_t* version);

} // namespace core