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
  read_success_and_failed_transaction_from_db_with_duration(
      block_height_t start_block, block_height_t end_block) = 0;
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
    const std::string aql =
        "for tx in transaction filter tx.status!=0 and tx.height>=" +
        std::to_string(start_block) +
        " and tx.height<=" + std::to_string(end_block) +
        " return {status:tx.status, from:tx.from, to:tx.to, "
        "tx_value:tx.tx_value, height:tx.height, timestamp:tx.timestamp, "
        "type_from:tx.type_from, type_to:tx.type_to, gas_used:tx.gas_used, "
        "gas_price:tx.gas_price, contract_address:tx.contract_address}";
    LOG(INFO) << aql;
    auto resp_ptr = this->aql_query(aql);
    return parse_from_response(std::move(resp_ptr));
  }

  virtual std::vector<transaction_info_t>
  read_success_and_failed_transaction_from_db_with_duration(
      block_height_t start_block, block_height_t end_block) {
    const std::string aql =
        "for tx in transaction filter tx.height>=" +
        std::to_string(start_block) +
        " and tx.height<=" + std::to_string(end_block) +
        " return {status:tx.status, from:tx.from, to:tx.to, "
        "tx_value:tx.tx_value, height:tx.height, timestamp:tx.timestamp, "
        "type_from:tx.type_from, type_to:tx.type_to, gas_used:tx.gas_used, "
        "gas_price:tx.gas_price, contract_address:tx.contract_address}";
    auto resp_ptr = this->aql_query(aql);
    return parse_from_response(std::move(resp_ptr));
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

}; // end class transaction_db
} // namespace neb
