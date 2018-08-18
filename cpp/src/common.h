#pragma once
#include <glog/logging.h>

#include <algorithm>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace neb {

typedef int64_t block_height_t;
typedef std::string account_address_t;
typedef std::string contract_address_t;
typedef boost::multiprecision::int128_t account_balance_t;
typedef int32_t account_type_t;

struct nebulas_db {};
struct eth_db {};

} // namespace neb
