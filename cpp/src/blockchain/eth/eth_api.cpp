#include "blockchain/eth/eth_api.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace neb {
namespace eth {

std::string json_parse_eth_balance(const std::string &json) {
  boost::property_tree::ptree pt;
  std::stringstream ss(json);

  try {
    boost::property_tree::read_json(ss, pt);
  } catch (boost::property_tree::ptree_error &e) {
    exit(-1);
  }

  return pt.get<std::string>("result");
}

std::string get_address_balance(const std::string &address,
                                const std::string &hex_height) {
  std::string cmd = boost::str(
      boost::format("curl -s --data "
                    "'{\"method\":\"eth_getBalance\",\"params\":[\"%1%\",\"%2%"
                    "\"],\"id\":1,\"jsonrpc\":\"2.0\"}' -H \"Content-Type: "
                    "application/json\" -X POST localhost:8545") %
      address % hex_height);
  std::string ret = get_stdout_from_command(cmd);
  std::vector<std::string> v = split_by_comma(ret, '\n');

  if (v.empty()) {
    LOG(INFO) << "eth get balance empty";
    exit(-1);
  }
  std::string resp = v.back();

  return json_parse_eth_balance(resp);
}

std::string json_parse_eth_code(const std::string &json) {
  boost::property_tree::ptree pt;
  std::stringstream ss(json);

  try {
    boost::property_tree::read_json(ss, pt);
  } catch (boost::property_tree::ptree_error &e) {
    exit(-1);
  }

  boost::property_tree::ptree::const_assoc_iterator it = pt.find("error");
  return it == pt.not_found() ? pt.get<std::string>("result")
                              : std::string("invalid");
}

std::string eth_get_code(const std::string &address,
                         const std::string hex_height) {
  std::string cmd = boost::str(
      boost::format("curl -s --data "
                    "'{\"method\":\"eth_getCode\",\"params\":[\"%1%\",\"%2%\"],"
                    "\"id\":1,\"jsonrpc\":\"2.0\"}' -H \"Content-Type: "
                    "application/json\" -X POST localhost:8545") %
      address % hex_height);
  std::string ret = get_stdout_from_command(cmd);
  std::vector<std::string> v = split_by_comma(ret, '\n');

  if (v.empty()) {
    LOG(INFO) << "eth get code empty";
    exit(-1);
  }
  std::string resp = v.back();

  return json_parse_eth_code(resp);
}

std::string get_address_type(const std::string &address,
                             const std::string &hex_height) {
  if (address.compare("none") == 0) {
    return std::string("none");
  }
  std::string eth_code = eth_get_code(address, hex_height);
  if (eth_code.compare("invalid") == 0) {
    return eth_code;
  }
  return eth_code.compare("0x") == 0 ? std::string("normal")
                                     : std::string("contract");
}

block_height_t json_parse_eth_block_number(const std::string &json) {
  boost::property_tree::ptree pt;
  std::stringstream ss(json);

  try {
    boost::property_tree::read_json(ss, pt);
  } catch (boost::property_tree::ptree_error &e) {
    exit(-1);
  }

  std::string height = pt.get<std::string>("result");
  return std::stoi(to_dec(height));
}

block_height_t get_block_height() {
  std::string cmd = std::string(
      "curl -s --data "
      "'{\"method\":\"eth_blockNumber\",\"params\":[],\"id\":1,\"jsonrpc\":\"2."
      "0\"}' -H \"Content-Type: application/json\" -X POST localhost:8545");
  std::string ret = get_stdout_from_command(cmd);
  std::vector<std::string> v = split_by_comma(ret, '\n');

  if (v.empty()) {
    LOG(INFO) << "get block height empty";
    exit(-1);
  }
  std::string resp = v.back();

  return json_parse_eth_block_number(resp);
}

std::vector<transaction_info_t>
json_parse_eth_block_transactions(const std::string &json) {
  boost::property_tree::ptree pt;
  std::stringstream ss(json);
  try {
    boost::property_tree::read_json(ss, pt);
  } catch (boost::property_tree::ptree_error &e) {
    exit(-1);
  }

  boost::property_tree::ptree result = pt.get_child("result");
  std::string timestamp = to_dec(result.get<std::string>("timestamp"));

  boost::property_tree::ptree transactions = result.get_child("transactions");
  std::vector<transaction_info_t> tx_v;

  BOOST_FOREACH (boost::property_tree::ptree::value_type &v, transactions) {
    boost::property_tree::ptree tx = v.second;
    std::string hash = tx.get<std::string>("hash");
    std::string gas_price = to_dec(tx.get<std::string>("gasPrice"));

    transaction_info_t info;
    info.template set<::neb::timestamp, ::neb::hash, ::neb::gas_price>(
        timestamp, hash, gas_price);
    tx_v.push_back(info);
  }

  if (tx_v.empty()) {
    transaction_info_t info;
    info.template set<::neb::timestamp>(timestamp);
    tx_v.push_back(info);
  }
  return tx_v;
}

std::vector<transaction_info_t>
get_block_transactions_by_height(block_height_t height) {
  std::string cmd = boost::str(
      boost::format("curl -s --data "
                    "'{\"method\":\"eth_getBlockByNumber\",\"params\":[\"%1%\","
                    "true],\"id\":1,\"jsonrpc\":\"2.0\"}' -H \"Content-Type: "
                    "application/json\" -X POST localhost:8545") %
      to_hex(std::to_string(height)));
  std::string ret = get_stdout_from_command(cmd);
  std::vector<std::string> v = split_by_comma(ret, '\n');

  if (v.empty()) {
    LOG(INFO) << "get block transactions by height empty";
    exit(-1);
  }
  std::string resp = v.back();

  return json_parse_eth_block_transactions(resp);
}

transaction_info_t parse_by_action_type(const boost::property_tree::ptree &pt) {

  transaction_info_t info;
  block_height_t height = pt.get<block_height_t>("blockNumber");
  std::string hash = pt.get<std::string>("transactionHash");
  std::string tx_type = pt.get<std::string>("type");

  boost::property_tree::ptree::const_assoc_iterator it = pt.find("error");
  int32_t status = (it == pt.not_found() ? 1 : 0);

  info.template set<::neb::height, ::neb::hash, ::neb::tx_type, ::neb::status>(
      height, hash, tx_type, status);

  boost::property_tree::ptree action = pt.get_child("action");

  auto f_create = [&]() {
    std::string from = action.get<std::string>("from");
    std::string to = from;
    std::string tx_value = to_dec(action.get<std::string>("value"));
    std::string data = action.get<std::string>("init");
    std::string gas_limit = to_dec(action.get<std::string>("gas"));
    info.template set<::neb::from, ::neb::to, ::neb::tx_value, ::neb::data,
                      ::neb::gas_limit>(from, to, tx_value, data, gas_limit);
    if (!status) {
      return;
    }
    boost::property_tree::ptree result = pt.get_child("result");
    std::string contract_address = result.get<std::string>("address");
    std::string gas_used = to_dec(result.get<std::string>("gasUsed"));
    info.template set<::neb::contract_address, ::neb::gas_used>(
        contract_address, gas_used);
  };
  auto f_call = [&]() {
    std::string from = action.get<std::string>("from");
    std::string to = action.get<std::string>("to");
    std::string tx_value = to_dec(action.get<std::string>("value"));
    std::string data = action.get<std::string>("input");
    std::string gas_limit = to_dec(action.get<std::string>("gas"));
    info.template set<::neb::from, ::neb::to, ::neb::tx_value, ::neb::data,
                      ::neb::gas_limit>(from, to, tx_value, data, gas_limit);
    if (!status) {
      return;
    }
    boost::property_tree::ptree result = pt.get_child("result");
    std::string gas_used = to_dec(result.get<std::string>("gasUsed"));
    info.template set<::neb::gas_used>(gas_used);
  };
  auto f_reward = [&]() {
    std::string from = "none";
    std::string to = action.get<std::string>("author");
    std::string tx_value = to_dec(action.get<std::string>("value"));
    info.template set<::neb::from, ::neb::to, ::neb::tx_value>(from, to,
                                                               tx_value);
  };
  auto f_suicide = [&]() {
    std::string from = action.get<std::string>("address");
    std::string to = action.get<std::string>("refundAddress");
    std::string tx_value = to_dec(action.get<std::string>("balance"));
    info.template set<::neb::from, ::neb::to, ::neb::tx_value>(from, to,
                                                               tx_value);
  };

  std::unordered_map<std::string, std::function<void()>> type_and_func(
      {{"create", f_create},
       {"call", f_call},
       {"reward", f_reward},
       {"suicide", f_suicide}});
  type_and_func.find(tx_type)->second();
  return info;
}

std::vector<transaction_info_t>
json_parse_trace_block(const std::string &json) {
  boost::property_tree::ptree pt;
  std::stringstream ss(json);
  try {
    boost::property_tree::read_json(ss, pt);
  } catch (boost::property_tree::ptree_error &e) {
    exit(-1);
  }

  boost::property_tree::ptree transactions = pt.get_child("result");
  std::vector<transaction_info_t> tx_v;

  BOOST_FOREACH (boost::property_tree::ptree::value_type &v, transactions) {
    boost::property_tree::ptree tx = v.second;
    tx_v.push_back(parse_by_action_type(tx));
  }
  return tx_v;
}

std::vector<transaction_info_t> trace_block(block_height_t height) {
  std::string cmd = boost::str(
      boost::format("curl -s --data "
                    "'{\"method\":\"trace_block\",\"params\":[\"%1%\"],\"id\":"
                    "1,\"jsonrpc\":\"2.0\"}' -H \"Content-Type: "
                    "application/json\" -X POST localhost:8545") %
      to_hex(std::to_string(height)));
  std::string ret = get_stdout_from_command(cmd);
  std::vector<std::string> v = split_by_comma(ret, '\n');

  if (v.empty()) {
    LOG(INFO) << "trace block empty";
    exit(-1);
  }
  std::string resp = v.back();

  return json_parse_trace_block(resp);
}

} // namespace eth
} // namespace neb
