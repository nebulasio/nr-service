#pragma once

#include "blockchain/db.h"
#include "sql/ntobject.h"
#include "sql/table.h"

namespace neb {

define_nt(tx_id, int64_t, "tx_id");
define_nt(nonce, std::string, "nonce");
define_nt(status, int32_t, "status");
define_nt(chainId, int32_t, "chainId");
define_nt(from, std::string, "from");
define_nt(timestamp, std::string, "timestamp");
define_nt(gas_used, std::string, "gas_used");
define_nt(tx_value, std::string, "value");
define_nt(data, std::string, "data");
define_nt(to, std::string, "to");
define_nt(contract_address, std::string, "contract_address");
define_nt(hash, std::string, "hash");
define_nt(gas_price, std::string, "gas_price");
define_nt(tx_type, std::string, "type");
define_nt(gas_limit, std::string, "gas_limit");
define_nt(height, int64_t, "height");
define_nt(type_from, std::string, "type_from");
define_nt(type_to, std::string, "type_to");

typedef ntarray<tx_id, nonce, status, chainId, from, timestamp, gas_used,
                tx_value, data, to, contract_address, hash, gas_price, tx_type,
                gas_limit, height, type_from, type_to>
    transaction_table_t;
typedef typename transaction_table_t::row_type transaction_info_t;

class transaction_db_interface {
public:
  virtual std::vector<transaction_info_t>
  read_transaction_simplified_from_db_with_duration(
      block_height_t start_block, block_height_t end_block) = 0;
  virtual std::vector<transaction_info_t>
  read_success_and_failed_transaction_from_db_with_block_duration(
      block_height_t start_block, block_height_t end_block) = 0;
  virtual std::vector<transaction_info_t>
  read_success_and_failed_transaction_from_db_with_ts_duration(
      const std::string &start_ts, const std::string &end_ts) = 0;
  virtual std::vector<transaction_info_t>
  read_success_and_failed_transaction_from_db_with_address(
      const std::string &address) = 0;
};

template <typename DB>
class transaction_db : public db<DB>, public transaction_db_interface {
public:
  transaction_db() : db<DB>() {}
  transaction_db(const std::string &url, const std::string &usrname,
                 const std::string &passwd, const std::string &dbname)
      : db<DB>(url, usrname, passwd, dbname) {}

  virtual std::vector<transaction_info_t>
  read_transaction_simplified_from_db_with_duration(block_height_t start_block,
                                                    block_height_t end_block) {
    const std::string aql = boost::str(
        boost::format(
            "for tx in transaction filter tx.status!=0 and tx.height>=%1% and "
            "tx.height<=%2% return {status:tx.status, from:tx.from, to:tx.to, "
            "tx_value:tx.tx_value, height:tx.height, timestamp:tx.timestamp, "
            "type_from:tx.type_from, type_to:tx.type_to, gas_used:tx.gas_used, "
            "gas_price:tx.gas_price, contract_address:tx.contract_address}") %
        start_block % end_block);
    auto resp_ptr = this->aql_query(aql);
    return parse_from_response(std::move(resp_ptr));
  }

  virtual std::vector<transaction_info_t>
  read_success_and_failed_transaction_from_db_with_block_duration(
      block_height_t start_block, block_height_t end_block) {
    const std::string aql = boost::str(
        boost::format(
            "for tx in transaction filter tx.height>=%1% and tx.height<=%2% "
            "return {tx_id:tx.tx_id, status:tx.status, from:tx.from, to:tx.to, "
            "tx_value:tx.tx_value, height:tx.height, timestamp:tx.timestamp, "
            "type_from:tx.type_from, type_to:tx.type_to, gas_used:tx.gas_used, "
            "gas_price:tx.gas_price, contract_address:tx.contract_address}") %
        start_block % end_block);
    auto resp_ptr = this->aql_query(aql);
    return parse_from_response(std::move(resp_ptr));
  }

  virtual std::vector<transaction_info_t>
  read_success_and_failed_transaction_from_db_with_ts_duration(
      const std::string &start_ts, const std::string &end_ts) {
    const std::string aql = boost::str(
        boost::format(
            "for tx in transaction filter tx.timestamp>='%1%' and "
            "tx.timestamp<='%2%' return {tx_id:tx.tx_id, status:tx.status, "
            "from:tx.from, to:tx.to, tx_value:tx.tx_value, height:tx.height, "
            "timestamp:tx.timestamp, type_from:tx.type_from, "
            "type_to:tx.type_to, gas_used:tx.gas_used, gas_price:tx.gas_price, "
            "contract_address:tx.contract_address}") %
        start_ts % end_ts);
    auto resp_ptr = this->aql_query(aql);
    return parse_from_response(std::move(resp_ptr));
  }

  virtual std::vector<transaction_info_t>
  read_success_and_failed_transaction_from_db_with_address(
      const std::string &address) {
    const std::string aql = boost::str(
        boost::format(
            "for tx in transaction filter tx.from=='%1%' or tx.to=='%1%'"
            "return {tx_id:tx.tx_id, status:tx.status, from:tx.from, to:tx.to, "
            "tx_value:tx.tx_value, height:tx.height, timestamp:tx.timestamp, "
            "type_from:tx.type_from, type_to:tx.type_to, gas_used:tx.gas_used, "
            "gas_price:tx.gas_price, contract_address:tx.contract_address}") %
        address);
    auto resp_ptr = this->aql_query(aql);
    return parse_from_response(std::move(resp_ptr));
  }

  std::string to_string(const std::vector<transaction_info_t> &rs) {
    boost::property_tree::ptree root;
    boost::property_tree::ptree arr;

    for (auto it = rs.begin(); it != rs.end(); it++) {
      const transaction_info_t &info = *it;
      int64_t status = info.template get<::neb::status>();
      std::string from = info.template get<::neb::from>();
      std::string to = info.template get<::neb::to>();
      std::string tx_value = info.template get<::neb::tx_value>();
      int64_t height = info.template get<::neb::height>();
      std::string timestamp = info.template get<::neb::timestamp>();
      std::string type_from = info.template get<::neb::type_from>();
      std::string type_to = info.template get<::neb::type_to>();
      std::string gas_used = info.template get<::neb::gas_used>();
      std::string gas_price = info.template get<::neb::gas_price>();
      std::string contract_address =
          info.template get<::neb::contract_address>();

      std::unordered_map<std::string, std::string> kv_pair(
          {{"status", std::to_string(status)},
           {"from", from},
           {"to", to},
           {"tx_value", tx_value},
           {"height", std::to_string(height)},
           {"timestamp", timestamp},
           {"type_from", type_from},
           {"type_to", type_to},
           {"gas_used", gas_used},
           {"gas_price", gas_price},
           {"contract_address", contract_address}});

      boost::property_tree::ptree p;
      for (auto &ele : kv_pair) {
        p.put(ele.first, ele.second);
      }

      arr.push_back(std::make_pair(std::string(), p));
    }
    root.add_child("transactions", arr);
    return ptree_to_string(root);
  }

private:
  void set_transaction_info(transaction_info_t &info, const VPackSlice &slice,
                            const std::string &key) {
    if (key.compare("status") == 0) {
      info.template set<::neb::status>(slice.getInt());
    }
    if (key.compare("from") == 0) {
      info.template set<::neb::from>(slice.copyString());
    }
    if (key.compare("to") == 0) {
      info.template set<::neb::to>(slice.copyString());
    }
    if (key.compare("tx_value") == 0) {
      info.template set<::neb::tx_value>(slice.copyString());
    }
    if (key.compare("height") == 0) {
      info.template set<::neb::height>(slice.getInt());
    }
    if (key.compare("timestamp") == 0) {
      info.template set<::neb::timestamp>(slice.copyString());
    }
    if (key.compare("type_from") == 0) {
      info.template set<::neb::type_from>(slice.copyString());
    }
    if (key.compare("type_to") == 0) {
      info.template set<::neb::type_to>(slice.copyString());
    }
    if (key.compare("gas_used") == 0) {
      info.template set<::neb::gas_used>(slice.copyString());
    }
    if (key.compare("gas_price") == 0) {
      info.template set<::neb::gas_price>(slice.copyString());
    }
    if (key.compare("contract_address") == 0) {
      info.template set<::neb::contract_address>(slice.copyString());
    }
  }

  std::vector<transaction_info_t> parse_from_response(
      const std::unique_ptr<::arangodb::fuerte::Response> resp_ptr) {
    std::vector<transaction_info_t> rs;
    auto documents = resp_ptr->slices().front().get("result");
    if (documents.isNone() || documents.isEmptyArray()) {
      return rs;
    }

    for (size_t i = 0; i < documents.length(); i++) {
      auto doc = documents.at(i);
      transaction_info_t info;
      for (size_t j = 0; j < doc.length(); j++) {
        std::string key = doc.keyAt(j).copyString();
        set_transaction_info(info, doc.valueAt(j), key);
      }
      rs.push_back(info);
    }
    return rs;
  }

  std::string ptree_to_string(const boost::property_tree::ptree &root) {
    std::stringstream ss;
    write_json(ss, root, false);
    LOG(INFO) << "write json: " << ss.str();
    return ss.str();
  }

}; // end class transaction_db
} // namespace neb
