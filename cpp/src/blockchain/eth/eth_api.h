#pragma once
#include "blockchain/transaction_db.h"
#include "common.h"
#include "utils/util.h"

namespace neb {
namespace eth {

std::string get_address_type(const std::string &address);

std::string get_address_balance(const std::string &address,
                                const std::string &hex_height);

block_height_t get_block_height();

std::vector<transaction_info_t>
get_block_transactions_by_height(block_height_t height);

std::vector<transaction_info_t> trace_block(block_height_t height);
}
} // namespace neb
