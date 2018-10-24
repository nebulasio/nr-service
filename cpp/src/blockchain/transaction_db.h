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
  virtual std::shared_ptr<std::vector<transaction_info_t>>
  read_inter_transaction_from_db_with_duration(block_height_t start_block,
                                               block_height_t end_block) = 0;
  virtual std::shared_ptr<std::vector<transaction_info_t>>
  read_success_and_failed_transaction_from_db_with_block_duration(
      block_height_t start_block, block_height_t end_block) = 0;

  virtual std::shared_ptr<std::vector<transaction_info_t>>
  read_success_and_failed_transaction_from_db_with_ts_duration(
      const std::string &start_ts, const std::string &end_ts) = 0;
  virtual std::shared_ptr<std::vector<transaction_info_t>>
  read_success_and_failed_transaction_from_db_with_address(
      const std::string &address) = 0;

  virtual std::string
  transaction_infos_to_string(const std::vector<transaction_info_t> &rs) = 0;
};

struct transaction_db_infosetter {
  typedef transaction_info_t info_type;

  static void set_info(transaction_info_t &info, const VPackSlice &slice,
                       const std::string &key) {
    if (key.compare("tx_id") == 0) {
      info.template set<::neb::tx_id>(std::stoi(slice.copyString()));
    }
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
    if (key.compare("tx_type") == 0) {
      info.template set<::neb::tx_type>(slice.copyString());
    }
  }
};

template <typename DB>
class transaction_db : public db<DB, transaction_db_infosetter>,
                       public transaction_db_interface {
public:
  typedef db<DB, transaction_db_infosetter> base_db_t;
  transaction_db() {}
  transaction_db(const std::string &url, const std::string &usrname,
                 const std::string &passwd, const std::string &dbname)
      : db<DB, transaction_db_infosetter>(url, usrname, passwd, dbname) {}

  virtual std::shared_ptr<std::vector<transaction_info_t>>
  read_inter_transaction_from_db_with_duration(block_height_t start_block,
                                               block_height_t end_block) {
    std::vector<transaction_info_t> ret;

    block_height_t block_interval = 1 << 6;
    for (auto s = start_block; s < end_block; s += block_interval) {
      auto e = s + block_interval;
      if (e > end_block) {
        auto v =
            read_inter_transaction_from_db_with_duration_sharding(s, end_block);
        ret.insert(ret.end(), v->begin(), v->end());
        break;
      }

      auto v = read_inter_transaction_from_db_with_duration_sharding(s, e);
      ret.insert(ret.end(), v->begin(), v->end());
    }
    return std::make_shared<std::vector<transaction_info_t>>(ret);
  }

  virtual std::shared_ptr<std::vector<transaction_info_t>>
  read_success_and_failed_transaction_from_db_with_block_duration(
      block_height_t start_block, block_height_t end_block) {

    std::vector<transaction_info_t> ret;

    block_height_t block_interval = 1 << 6;
    for (auto s = start_block; s < end_block; s += block_interval) {
      auto e = s + block_interval;
      if (e > end_block) {
        auto v =
            read_success_and_failed_transaction_from_db_with_block_duration_sharding(
                s, end_block);
        ret.insert(ret.end(), v->begin(), v->end());
        break;
      }

      auto v =
          read_success_and_failed_transaction_from_db_with_block_duration_sharding(
              s, e);
      ret.insert(ret.end(), v->begin(), v->end());
    }
    return std::make_shared<std::vector<transaction_info_t>>(ret);
  }

  virtual std::shared_ptr<std::vector<transaction_info_t>>
  read_success_and_failed_transaction_from_db_with_ts_duration(
      const std::string &start_ts, const std::string &end_ts) {

    std::vector<transaction_info_t> ret;

    block_height_t block_interval = 1 << 6;
    std::string ts_interval = std::to_string(block_interval * 15);
    for (auto s = start_ts; s < end_ts;
         s = std::to_string(std::stoi(s) + std::stoi(ts_interval))) {
      auto e = std::to_string(std::stoi(s) + std::stoi(ts_interval));
      if (e.compare(end_ts) > 0) {
        auto v =
            read_success_and_failed_transaction_from_db_with_ts_duration_sharding(
                s, end_ts);
        ret.insert(ret.end(), v->begin(), v->end());
        break;
      }

      auto v =
          read_success_and_failed_transaction_from_db_with_ts_duration_sharding(
              s, e);
      ret.insert(ret.end(), v->begin(), v->end());
    }
    return std::make_shared<std::vector<transaction_info_t>>(ret);
  }

  virtual std::shared_ptr<std::vector<transaction_info_t>>
  read_success_and_failed_transaction_from_db_with_address(
      const std::string &address) {
    const std::string aql = boost::str(
        boost::format(
            "for tx in transaction filter tx.from=='%1%' or tx.to=='%1%' "
            "return {tx_id:tx._key, status:tx.status, from:tx.from, to:tx.to, "
            "tx_value:tx.tx_value, height:tx.height, timestamp:tx.timestamp, "
            "type_from:tx.type_from, type_to:tx.type_to, gas_used:tx.gas_used, "
            "gas_price:tx.gas_price, contract_address:tx.contract_address, "
            "tx_type:tx.tx_type}") %
        address);
    auto resp_ptr = this->aql_query(aql);
    return from_response(std::move(resp_ptr));
  }

  virtual void
  remove_success_and_failed_transaction_from_db_with_block_duration(
      block_height_t start_block, block_height_t end_block) {

    auto it_txs =
        read_success_and_failed_transaction_from_db_with_block_duration(
            start_block, end_block);
    auto txs = *it_txs;
    LOG(INFO) << "txs size: " << txs.size();

    for (auto tx : txs) {
      int64_t tx_id = tx.template get<::neb::tx_id>();

      const std::string rm_from_tx = boost::str(
          boost::format("for tx in from_txs filter tx._to=='transaction/%1%' "
                        "remove tx in from_txs") %
          tx_id);
      this->aql_query(rm_from_tx);

      const std::string rm_tx_to = boost::str(
          boost::format("for tx in txs_to filter tx._from=='transaction/%1%' "
                        "remove tx in txs_to") %
          tx_id);
      this->aql_query(rm_tx_to);

      const std::string rm_height_tx = boost::str(
          boost::format("for tx in height_txs filter tx._to=='transaction/%1%' "
                        "remove tx in height_txs") %
          tx_id);
      this->aql_query(rm_height_tx);
    }

    const std::string rm_txs = boost::str(
        boost::format("for tx in transaction filter tx.height>=%1% and "
                      "tx.height<=%2% remove tx in transaction") %
        start_block % end_block);
    this->aql_query(rm_txs);
  }

  virtual std::string
  transaction_infos_to_string(const std::vector<transaction_info_t> &rs) {
    boost::property_tree::ptree root;
    boost::property_tree::ptree arr;

    for (auto it = rs.begin(); it != rs.end(); it++) {
      const transaction_info_t &info = *it;
      boost::property_tree::ptree p;
      convert_transaction_info_to_ptree(info, p);
      arr.push_back(std::make_pair(std::string(), p));
    }
    root.add_child("transactions", arr);
    return base_db_t::ptree_to_string(root);
  }

private:
  static void
  convert_transaction_info_to_ptree(const transaction_info_t &info,
                                    boost::property_tree::ptree &p) {
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
    std::string contract_address = info.template get<::neb::contract_address>();
    std::string tx_type = info.template get<::neb::tx_type>();

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
         {"contract_address", contract_address},
         {"tx_type", tx_type}});

    for (auto &ele : kv_pair) {
      p.put(ele.first, ele.second);
    }
  }

  static std::shared_ptr<std::vector<transaction_info_t>>
  from_response(std::unique_ptr<::arangodb::fuerte::Response> resp_ptr) {
    std::vector<transaction_info_t> rs;
    base_db_t::parse_from_response(std::move(resp_ptr), rs);
    return std::make_shared<std::vector<neb::transaction_info_t>>(rs);
  }

  std::shared_ptr<std::vector<transaction_info_t>>
  read_inter_transaction_from_db_with_duration_sharding(
      block_height_t start_block, block_height_t end_block) {

    const std::string aql = boost::str(
        boost::format(
            "for tx in transaction filter tx.status!=0 and "
            "tx.type_from=='normal' and tx.type_to=='normal' and "
            "tx.height>=%1% and tx.height<%2% return {tx_id:tx._key, "
            "status:tx.status, from:tx.from, to:tx.to, tx_value:tx.tx_value, "
            "height:tx.height, timestamp:tx.timestamp, type_from:tx.type_from, "
            "type_to:tx.type_to, gas_used:tx.gas_used, gas_price:tx.gas_price, "
            "contract_address:tx.contract_address, tx_type:tx.tx_type}") %
        start_block % end_block);
    auto resp_ptr = this->aql_query(aql);
    return from_response(std::move(resp_ptr));
  }

  std::shared_ptr<std::vector<transaction_info_t>>
  read_success_and_failed_transaction_from_db_with_block_duration_sharding(
      block_height_t start_block, block_height_t end_block) {
    const std::string aql = boost::str(
        boost::format(
            "for tx in transaction filter tx.height>=%1% and tx.height<%2% "
            "return {tx_id:tx._key, status:tx.status, from:tx.from, to:tx.to, "
            "tx_value:tx.tx_value, height:tx.height, timestamp:tx.timestamp, "
            "type_from:tx.type_from, type_to:tx.type_to, gas_used:tx.gas_used, "
            "gas_price:tx.gas_price, contract_address:tx.contract_address, "
            "tx_type:tx.tx_type}") %
        start_block % end_block);
    auto resp_ptr = this->aql_query(aql);
    return from_response(std::move(resp_ptr));
  }

  std::shared_ptr<std::vector<transaction_info_t>>
  read_success_and_failed_transaction_from_db_with_ts_duration_sharding(
      const std::string &start_ts, const std::string &end_ts) {
    const std::string aql = boost::str(
        boost::format(
            "for tx in transaction filter tx.timestamp>='%1%' and "
            "tx.timestamp<'%2%' return {tx_id:tx._key, status:tx.status, "
            "from:tx.from, to:tx.to, tx_value:tx.tx_value, height:tx.height, "
            "timestamp:tx.timestamp, type_from:tx.type_from, "
            "type_to:tx.type_to, gas_used:tx.gas_used, gas_price:tx.gas_price, "
            "contract_address:tx.contract_address, tx_type:tx.tx_type}") %
        start_ts % end_ts);
    auto resp_ptr = this->aql_query(aql);
    return from_response(std::move(resp_ptr));
  }

}; // end class transaction_db
} // namespace neb
