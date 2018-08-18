#pragma once

#include "common.h"
#include <boost/graph/adjacency_list.hpp>
#include <cstdint>
#include <queue>
#include <vector>

namespace boost {
enum edge_timestamp_t { edge_timestamp };

BOOST_INSTALL_PROPERTY(edge, timestamp);
} // namespace boost
namespace neb {}
