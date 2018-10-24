#pragma once
#include "blockchain/transaction_db.h"
#include "common.h"
#include "utils/util.h"

namespace neb {
namespace eth {

class eth_api {
public:
  static std::string get_address_balance(const std::string &address,
                                         const std::string &hex_height);

  static std::string get_address_type(const std::string &address,
                                      const std::string &hex_height);

  static block_height_t get_block_height();

  static std::shared_ptr<std::vector<transaction_info_t>>
  get_block_transactions_by_height(block_height_t height);

  static std::shared_ptr<std::vector<transaction_info_t>>
  trace_block(block_height_t height);

private:
  static std::string json_parse_eth_balance(const std::string &json);

  static std::string json_parse_eth_code(const std::string &json);
  static std::string eth_get_code(const std::string &address,
                                  const std::string hex_height);

  static block_height_t json_parse_eth_block_number(const std::string &json);

  static std::shared_ptr<std::vector<transaction_info_t>>
  json_parse_eth_block_transactions(const std::string &json);

  static std::shared_ptr<transaction_info_t>
  parse_by_action_type(const boost::property_tree::ptree &pt);
  static std::shared_ptr<std::vector<transaction_info_t>>
  json_parse_trace_block(const std::string &json);
};

} // namespace eth
} // namespace neb
