#pragma once

#include "apiserver.h"
#include "blockchain.h"
#include "err/err_def.h"

typedef neb::lru_cache<neb::block_height_t,
                       std::shared_ptr<std::vector<neb::transaction_info_t>>>
    height_transaction_cache_t;
typedef neb::lru_cache<std::string,
                       std::shared_ptr<std::vector<neb::transaction_info_t>>>
    address_transaction_cache_t;
typedef neb::transaction_db_interface transaction_db_t;
typedef std::shared_ptr<transaction_db_t> transaction_db_ptr_t;

template <typename TS> struct transaction_apiserver_traits {};
template <> struct transaction_apiserver_traits<neb::nebulas_db> {
  static transaction_db_ptr_t get_transaction_db_ptr() {
    typedef neb::transaction_db<neb::nebulas_db> nebulas_transaction_db_t;
    return std::make_shared<nebulas_transaction_db_t>(
        std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
        std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  }
  static bool is_address_valid(const std::string &address) {
    return ::neb::nebulas::nebulas_api::is_contract_address(address) >= 0;
  }
};
template <> struct transaction_apiserver_traits<neb::eth_db> {
  static transaction_db_ptr_t get_transaction_db_ptr() {
    typedef neb::transaction_db<neb::eth_db> eth_transaction_db_t;
    return std::make_shared<eth_transaction_db_t>(
        std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
        std::getenv("DB_PASSWORD"), std::getenv("ETH_DB"));
  }
  static bool is_address_valid(const std::string &address) {
    std::string type = ::neb::eth::eth_api::get_address_type(address, "0x0");
    return type.compare("invalid") != 0 && type.compare("none") != 0;
  }
};

template <class DB> class transaction_apiserver : public apiserver {
public:
  transaction_apiserver(const std::string &appname) : apiserver(appname) {

    m_height_transaction_cache_ptr =
        std::unique_ptr<height_transaction_cache_t>(
            new height_transaction_cache_t());
    m_address_transaction_cache_ptr =
        std::unique_ptr<address_transaction_cache_t>(
            new address_transaction_cache_t());
    m_tx_ptr = transaction_apiserver_traits<DB>::get_transaction_db_ptr();
  }

  std::string on_api_transaction(
      const std::unordered_map<std::string, std::string> &params) {

    if (has_keys_in_params(
            std::unordered_set<std::string>({"start_block", "end_block"}),
            params)) {
      return on_api_height_transaction(params);
    }
    if (has_keys_in_params(std::unordered_set<std::string>({"address"}),
                           params)) {
      return on_api_address_transaction(params);
    }
    LOG(WARNING)
        << "params not matched, no param \'start_block\' or \'end_block\'";
    return err_code_params_not_matched;
  }

private:
  bool has_keys_in_params(
      const std::unordered_set<std::string> &keys,
      const std::unordered_map<std::string, std::string> &params) {

    for (auto it = params.begin(); it != params.end(); it++) {
      if (keys.find(it->first) == keys.end()) {
        return false;
      }
    }
    return true;
  }

  std::string on_api_height_transaction(
      const std::unordered_map<std::string, std::string> &params) {

    LOG(INFO) << "on api height transaction, request params";
    for (auto it = params.begin(); it != params.end(); it++) {
      LOG(INFO) << it->first << ',' << it->second;
    }

    if (params.find("start_block") == params.end() ||
        params.find("end_block") == params.end()) {
      LOG(WARNING)
          << "params not matched, no param \'start_block\' or \'end_block\'";
      return err_code_params_not_matched;
    }

    std::string s_start_block = params.find("start_block")->second;
    std::string s_end_block = params.find("end_block")->second;
    if (!neb::string_utils::is_number(s_start_block) ||
        !neb::string_utils::is_number(s_end_block)) {
      LOG(WARNING) << "params value invalid, start_block/end_block value "
                      "contains unexpected character";
      return err_code_params_value_invalid;
    }

    neb::block_height_t start_block = std::stoi(s_start_block);
    neb::block_height_t end_block = std::stoi(s_end_block);

    height_transaction_cache_t &cache = *m_height_transaction_cache_ptr;
    std::shared_ptr<std::vector<neb::transaction_info_t>> txs =
        std::make_shared<std::vector<neb::transaction_info_t>>();

    for (neb::block_height_t h = start_block; h <= end_block; h++) {
      std::shared_ptr<std::vector<neb::transaction_info_t>> rs =
          std::make_shared<std::vector<neb::transaction_info_t>>();
      if (!cache.get(h, rs)) {
        LOG(INFO) << "transaction cache missed, reading from db";
        set_height_transaction_cache(cache, h);

        if (!cache.get(h, rs)) {
          LOG(INFO) << "transaction db missed";
        }
      }
      txs->insert(txs->end(), rs->begin(), rs->end());
    }
    LOG(INFO) << "transaction size: " << txs->size();

    return m_tx_ptr->transaction_infos_to_string(*txs);
  }

  std::string on_api_address_transaction(
      const std::unordered_map<std::string, std::string> &params) {

    LOG(INFO) << "on api address transaction, request params";
    for (auto it = params.begin(); it != params.end(); it++) {
      LOG(INFO) << it->first << ',' << it->second;
    }

    if (params.find("address") == params.end()) {
      LOG(WARNING) << "params not matched, no param \'address\'";
      return err_code_params_not_matched;
    }
    std::string address = params.find("address")->second;
    if (!transaction_apiserver_traits<DB>::is_address_valid(address) < 0) {
      LOG(WARNING)
          << "params value invalid, see nebulas address "
             "design, https://github.com/nebulasio/wiki/blob/master/address.md";
      return err_code_params_value_invalid;
    }

    address_transaction_cache_t &cache = *m_address_transaction_cache_ptr;
    std::shared_ptr<std::vector<neb::transaction_info_t>> rs =
        std::make_shared<std::vector<neb::transaction_info_t>>();
    if (!cache.get(address, rs)) {
      LOG(INFO) << "transaction cache missed, reading from db";
      set_address_transaction_cache(cache, address);

      if (!cache.get(address, rs)) {
        LOG(INFO) << "transaction db missed";
      }
    }
    LOG(INFO) << "transaction size: " << rs->size();

    return m_tx_ptr->transaction_infos_to_string(*rs);
  }

  void set_height_transaction_cache(height_transaction_cache_t &cache,
                                    neb::block_height_t height) {

    int32_t read_ahead_height = 3600 / 15;
    auto it_txs =
        m_tx_ptr
            ->read_success_and_failed_transaction_from_db_with_block_duration(
                height, height + read_ahead_height);
    auto txs = *it_txs;
    LOG(INFO) << "read ahead transaction size: " << txs.size();
    std::unordered_map<neb::block_height_t,
                       std::vector<neb::transaction_info_t>>
        height_and_txs;

    for (auto it = txs.begin(); it != txs.end(); it++) {
      neb::block_height_t height = it->template get<::neb::height>();
      height_and_txs[height].push_back(*it);
    }

    for (neb::block_height_t h = height; h <= height + read_ahead_height; h++) {
      auto it = height_and_txs.find(h);
      if (it != height_and_txs.end()) {
        cache.set(h, std::make_shared<std::vector<neb::transaction_info_t>>(
                         it->second));
      } else {
        cache.set(h, std::make_shared<std::vector<neb::transaction_info_t>>());
      }
    }
    return;
  }

  void set_address_transaction_cache(address_transaction_cache_t &cache,
                                     const std::string &address) {

    auto it_rs =
        m_tx_ptr->read_success_and_failed_transaction_from_db_with_address(
            address);
    auto rs = *it_rs;
    LOG(INFO) << "read transaction by address, size: " << rs.size();
    cache.set(address,
              std::make_shared<std::vector<neb::transaction_info_t>>(rs));
    return;
  }

protected:
  std::unique_ptr<height_transaction_cache_t> m_height_transaction_cache_ptr;
  std::unique_ptr<address_transaction_cache_t> m_address_transaction_cache_ptr;
  transaction_db_ptr_t m_tx_ptr;
};

