#pragma once

#include <set>
#include "status.h"

namespace core {

/// Obtains a set of gpu ids that is supported by triton.
/// \param supported_gpus Returns the set of integers which is
///  populated by ids of supported GPUS
/// \param min_compute_capability The minimum support CUDA compute
/// capability.
/// \return The error status. A non-ok status means there were
/// errors encountered while querying GPU devices.
Status GetSupportedGPUs(std::set<int>* supported_gpus, 
                        const double min_compute_capability);

}