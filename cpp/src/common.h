#pragma once
#include <glog/logging.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <boost/asio.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace neb {

typedef int64_t block_height_t;
typedef std::string account_address_t;
typedef std::string contract_address_t;
typedef boost::multiprecision::int128_t account_balance_t;
typedef int32_t account_type_t;

struct nebulas_db {};
struct eth_db {};

} // namespace neb
