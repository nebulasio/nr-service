#include "blockchain/nebulas/nebulas_api.h"
#include "utils/base58.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace neb {
namespace nebulas {

block_height_t json_parse_neb_state(const std::string &json) {
  boost::property_tree::ptree pt;
  std::stringstream ss(json);

  try {
    boost::property_tree::read_json(ss, pt);
  } catch (boost::property_tree::ptree_error &e) {
    return -1;
  }

  boost::property_tree::ptree result = pt.get_child("result");
  std::string height = result.get<std::string>("height");
  return std::stoi(height);
}

block_height_t get_block_height() {
  std::string cmd = "curl -s -H 'Content-Type: application/json' -X GET "
                    "http://localhost:8685/v1/user/nebstate";
  std::string ret = get_stdout_from_command(cmd);
  std::vector<std::string> v = split_by_comma(ret, '\n');

  if (v.size() < 1) {
    return -1;
  }
  std::string resp = v[v.size() - 1];

  return json_parse_neb_state(resp);
}

std::pair<std::string, int> json_parse_account_state(const std::string &json) {
  boost::property_tree::ptree pt;
  std::stringstream ss(json);

  try {
    boost::property_tree::read_json(ss, pt);
  } catch (boost::property_tree::ptree_error &e) {
    return std::make_pair("", -1);
  }

  boost::property_tree::ptree result = pt.get_child("result");
  std::string balance = result.get<std::string>("balance");
  int type = result.get<int>("type");
  return std::make_pair(balance, type);
}

std::pair<std::string, int> get_account_state(block_height_t height,
                                              const std::string &address) {
  std::string cmd =
      "curl -s -H 'Content-Type: application/json' -X POST "
      "http://localhost:8685/v1/user/accountstate -d '{\"height\": " +
      std::to_string(height) + ", \"address\": \"" + address + "\"}'";
  if (height == 0) {
    cmd = "curl -s -H 'Content-Type: application/json' -X POST "
          "http://localhost:8685/v1/user/accountstate -d '{\"address\": \"" +
          address + "\"}'";
  }
  // std::cout << cmd << std::endl;
  std::string ret = get_stdout_from_command(cmd);
  std::vector<std::string> v = split_by_comma(ret, '\n');

  if (v.size() < 1) {
    return std::make_pair("", -1);
  }
  std::string resp = v[v.size() - 1];

  return json_parse_account_state(resp);
}

int32_t is_contract_address(const std::string &address) {
  // int type = get_account_state(0, address).second;
  // decode base58 instead of rpc calling
  std::vector<unsigned char> v;
  /*
   * 0x57 stands for normal account, 0x58 for contract
   * +----------------------------
   * |  0x19  |  0x57 or 0x58
   * +---------------------------
   */
  bool ret = decode_base58(address, v);

  if (!ret) {
    // decode failed
    return -1;
  }

  if (v.size() < 2) {
    // must at least padding and type code
    return -1;
  }

  if (address.length() != 35) {
    // address has 35 chars
    return -1;
  }

  if (v[0] != 0x19) {
    // one byte fixed padding
    return -1;
  }

  if (v[1] != 0x57 && v[1] != 0x58) {
    // one byte type code
    return -1;
  }
  return v[1] == 0x57 ? 0 : 1;
}

std::vector<transaction_info_t>
json_parse_block_transactions(const std::string &json,
                              const std::string &block_timestamp) {
  boost::property_tree::ptree pt;
  std::stringstream ss(json);
  try {
    boost::property_tree::read_json(ss, pt);
  } catch (boost::property_tree::ptree_error &e) {
    return std::vector<transaction_info_t>();
  }

  boost::property_tree::ptree result = pt.get_child("result");
  bool is_finality = result.get<bool>("is_finality");
  // check finality
  if (!is_finality) {
    return std::vector<transaction_info_t>();
  }

  block_height_t height = result.get<block_height_t>("height");
  // std::cout << "height: "  << height << std::endl;

  boost::property_tree::ptree transactions = result.get_child("transactions");
  std::vector<transaction_info_t> tx_v;

  BOOST_FOREACH (boost::property_tree::ptree::value_type &v, transactions) {
    boost::property_tree::ptree tx = v.second;
    std::string nonce = tx.get<std::string>("nonce");
    int32_t status = tx.get<int32_t>("status");
    int32_t chain_id = tx.get<int32_t>("chainId");
    std::string from = tx.get<std::string>("from");
    std::string timestamp = block_timestamp;
    std::string gas_used = tx.get<std::string>("gas_used");
    std::string value = tx.get<std::string>("value");
    std::string data = tx.get<std::string>("data");
    std::string to = tx.get<std::string>("to");
    std::string contract_address = tx.get<std::string>("contract_address");
    std::string hash = tx.get<std::string>("hash");
    std::string gas_price = tx.get<std::string>("gas_price");
    std::string type = tx.get<std::string>("type");
    std::string gas_limit = tx.get<std::string>("gas_limit");

    int32_t ret_from = is_contract_address(from);
    std::string type_from =
        (ret_from == -1 ? "illegal" : (ret_from == 0 ? "normal" : "contract"));
    int32_t ret_to = is_contract_address(to);
    std::string type_to =
        (ret_to == -1 ? "illegal" : (ret_to == 0 ? "normal" : "contract"));

    transaction_info_t info;
    info.template set<::neb::nonce, ::neb::status, ::neb::chainId, ::neb::from,
                      ::neb::timestamp, ::neb::gas_used, ::neb::tx_value,
                      ::neb::data, ::neb::to, ::neb::contract_address,
                      ::neb::hash, ::neb::gas_price, ::neb::tx_type,
                      ::neb::gas_limit, ::neb::height, ::neb::type_from,
                      ::neb::type_to>(nonce, status, chain_id, from, timestamp,
                                      gas_used, value, data, to,
                                      contract_address, hash, gas_price, type,
                                      gas_limit, height, type_from, type_to);
    tx_v.push_back(info);
  }
  return tx_v;
}

std::vector<transaction_info_t>
get_block_transactions_by_height(block_height_t height,
                                 const std::string &block_timestamp) {
  std::string cmd =
      "curl -s -H 'Content-Type: application/json' -X POST "
      "http://localhost:8685/v1/user/getBlockByHeight -d '{\"height\": " +
      std::to_string(height) + ", \"full_fill_transaction\": true}'";
  std::string ret = get_stdout_from_command(cmd);
  std::vector<std::string> v = split_by_comma(ret, '\n');

  if (v.size() < 1) {
    return std::vector<transaction_info_t>();
  }
  std::string resp = v[v.size() - 1];

  return json_parse_block_transactions(resp, block_timestamp);
}

std::string json_parse_block_timestamp(const std::string &json) {
  boost::property_tree::ptree pt;
  std::stringstream ss(json);
  try {
    boost::property_tree::read_json(ss, pt);
  } catch (boost::property_tree::ptree_error &e) {
    return std::string();
  }

  boost::property_tree::ptree result = pt.get_child("result");
  bool is_finality = result.get<bool>("is_finality");
  // check finality
  if (!is_finality) {
    return std::string();
  }

  std::string timestamp = result.get<std::string>("timestamp");
  return timestamp;
}

std::string get_block_timestamp_by_height(block_height_t height) {
  std::string cmd =
      "curl -s -H 'Content-Type: application/json' -X POST "
      "http://localhost:8685/v1/user/getBlockByHeight -d '{\"height\": " +
      std::to_string(height) + ", \"full_fill_transaction\": true}'";
  std::string ret = get_stdout_from_command(cmd);
  std::vector<std::string> v = split_by_comma(ret, '\n');

  if (v.size() < 1) {
    return std::string();
  }
  std::string resp = v[v.size() - 1];

  return json_parse_block_timestamp(resp);
}

std::vector<event_t> json_parse_events(const std::string &json,
                                       int32_t tx_status) {
  boost::property_tree::ptree pt;
  std::stringstream ss(json);
  try {
    boost::property_tree::read_json(ss, pt);
  } catch (boost::property_tree::ptree_error &e) {
    return std::vector<event_t>();
  }

  boost::property_tree::ptree result = pt.get_child("result");
  boost::property_tree::ptree events = result.get_child("events");
  std::vector<event_t> ev_v;

  BOOST_FOREACH (boost::property_tree::ptree::value_type &v, events) {
    boost::property_tree::ptree event = v.second;
    std::string topic = event.get<std::string>("topic");
    if (topic.compare("chain.transferFromContract") != 0) {
      continue;
    }

    std::string data_json = event.get<std::string>("data");
    std::stringstream ss(data_json);
    try {
      boost::property_tree::read_json(ss, pt);
    } catch (boost::property_tree::ptree_error &e) {
      return std::vector<event_t>();
    }

    boost::property_tree::ptree::const_assoc_iterator it = pt.find("status");
    // event failed if status exists, status = 0
    int32_t status = tx_status;
    if (status != 0 && it != pt.not_found()) {
      status = 0;
    }
    std::string from = pt.get<std::string>("from");
    std::string to = pt.get<std::string>("to");
    // database tx_value field max length, in case of insufficient balance
    std::string amount = pt.get<std::string>("amount");
    ev_v.push_back(event_t{status, from, to, amount});
    // std::cout << from << ", " << to << ", " << amount << std::endl;
  }

  return ev_v;
}

std::vector<event_t> get_events_by_hash(const std::string &hash,
                                        int32_t tx_status) {
  std::string cmd =
      "curl -s -H 'Content-Type: application/json' -X POST "
      "http://localhost:8685/v1/user/getEventsByHash -d '{\"hash\":\"" +
      hash + "\"}'";
  std::string ret = get_stdout_from_command(cmd);
  std::vector<std::string> v = split_by_comma(ret, '\n');

  if (v.size() < 1) {
    return std::vector<event_t>();
  }
  std::string resp = v[v.size() - 1];
  // std::cout << resp << std::endl;

  return json_parse_events(resp, tx_status);
}

std::vector<transaction_info_t>
get_transaction_events(const transaction_info_t &transaction,
                       const std::string &block_timestamp, int32_t tx_status) {
  std::vector<transaction_info_t> transactions;

  std::string hash = transaction.template get<::neb::hash>();
  std::vector<event_t> events =
      neb::nebulas::get_events_by_hash(hash, tx_status);

  int chainId = transaction.template get<::neb::chainId>();
  std::string timestamp = block_timestamp;
  std::string type = "event";
  block_height_t height = transaction.template get<::neb::height>();
  std::string gas_used = std::string();

  for (auto it = events.begin(); it != events.end(); it++) {
    int32_t status = it->m_status;
    std::string from = it->m_from;
    std::string value = it->m_value;
    std::string to = it->m_to;

    int32_t ret_from = is_contract_address(from);
    std::string type_from =
        (ret_from == -1 ? "illegal" : (ret_from == 0 ? "normal" : "contract"));
    int32_t ret_to = is_contract_address(to);
    std::string type_to =
        (ret_to == -1 ? "illegal" : (ret_to == 0 ? "normal" : "contract"));

    transaction_info_t info;
    info.template set<::neb::status, ::neb::chainId, ::neb::from,
                      ::neb::timestamp, ::neb::tx_value, ::neb::to, ::neb::hash,
                      ::neb::tx_type, ::neb::height, ::neb::type_from,
                      ::neb::type_to, ::neb::gas_used>(
        status, chainId, from, timestamp, value, to, hash, type, height,
        type_from, type_to, gas_used);
    transactions.push_back(info);
  }
  return transactions;
}

} // namespace nebulas
} // namespace neb
