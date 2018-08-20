#pragma once
#include "blockchain/transaction_db.h"
#include "common.h"
#include "utils/util.h"

namespace neb {
namespace nebulas {

std::pair<std::string, int> get_account_state(block_height_t height,
                                              const std::string &address);
int32_t is_contract_address(const std::string &address);

block_height_t get_block_height();

std::vector<transaction_info_t>
get_block_transactions_by_height(block_height_t height,
                                 const std::string &block_timestamp);

std::string get_block_timestamp_by_height(block_height_t height);

std::vector<transaction_info_t>
get_transaction_events(const transaction_info_t &transaction,
                       const std::string &block_timestamp, int32_t tx_status);

struct event_t {
  int32_t m_status;
  std::string m_from;
  std::string m_to;
  std::string m_value;
};

std::vector<event_t> get_events_by_hash(const std::string &hash,
                                        int32_t tx_status);

} // namespace nebulas
} // namespace neb
