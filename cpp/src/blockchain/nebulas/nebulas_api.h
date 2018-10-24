#pragma once
#include "blockchain/transaction_db.h"
#include "common.h"
#include "utils/util.h"

namespace neb {
namespace nebulas {

struct event_t {
  int32_t m_status;
  std::string m_from;
  std::string m_to;
  std::string m_value;
};

class nebulas_api {
public:
  static std::pair<std::string, int>
  get_account_state(const std::string &address, block_height_t height);
  static int32_t is_contract_address(const std::string &address);

  static block_height_t get_block_height();

  static std::shared_ptr<std::vector<transaction_info_t>>
  get_block_transactions_by_height(block_height_t height,
                                   const std::string &block_timestamp);

  static std::string get_block_timestamp_by_height(block_height_t height);

  static std::shared_ptr<std::vector<transaction_info_t>>
  get_transaction_events(const transaction_info_t &transaction,
                         const std::string &block_timestamp, int32_t tx_status);

private:
  static std::pair<std::string, int>
  json_parse_account_state(const std::string &json);

  static block_height_t json_parse_neb_state(const std::string &json);

  static std::shared_ptr<std::vector<transaction_info_t>>
  json_parse_block_transactions(const std::string &json,
                                const std::string &block_timestamp);

  static std::string json_parse_block_timestamp(const std::string &json);

  static std::shared_ptr<std::vector<event_t>>
  json_parse_events(const std::string &json, int32_t tx_status);

  static std::shared_ptr<std::vector<event_t>>
  get_events_by_hash(const std::string &hash, int32_t tx_status);
};

} // namespace nebulas
} // namespace neb
