#pragma once

#include "apiserver.h"
#include "blockchain.h"
#include "err/err_def.h"

typedef neb::lru_cache<std::string, std::shared_ptr<neb::account_info_t>>
    account_cache_t;
typedef neb::account_db_interface account_db_t;
typedef std::shared_ptr<account_db_t> account_db_ptr_t;

template <typename TS> struct account_apiserver_traits {};
template <> struct account_apiserver_traits<neb::nebulas_db> {
  static account_db_ptr_t get_account_db_ptr() {
    typedef neb::account_db<neb::nebulas_db> nebulas_account_db_t;
    return std::make_shared<nebulas_account_db_t>(
        std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
        std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  }
  static bool is_address_valid(const std::string &address) {
    return ::neb::nebulas::is_contract_address(address) >= 0;
  }
};
template <> struct account_apiserver_traits<neb::eth_db> {
  static account_db_ptr_t get_account_db_ptr() {
    typedef neb::account_db<neb::eth_db> eth_account_db_t;
    return std::make_shared<eth_account_db_t>(
        std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
        std::getenv("DB_PASSWORD"), std::getenv("ETH_DB"));
  }
  static bool is_address_valid(const std::string &address) {
    std::string address_type = ::neb::eth::get_address_type(address);
    return address_type.compare("normal") == 0 ||
           address_type.compare("contract") == 0;
  }
};

template <class DB> class account_apiserver : public apiserver {
public:
  account_apiserver(const std::string &appname) : apiserver(appname) {
    m_cache_ptr = std::unique_ptr<account_cache_t>(new account_cache_t());
    m_ac_ptr = account_apiserver_traits<DB>::get_account_db_ptr();
  }

  std::string
  on_api_account(const std::unordered_map<std::string, std::string> &params) {
    LOG(INFO) << "on api account, request params";
    for (auto it = params.begin(); it != params.end(); it++) {
      LOG(INFO) << it->first << ',' << it->second;
    }

    if (params.find("address") == params.end()) {
      LOG(WARNING) << "params not matched, no param \'address\'";
      return err_code_params_not_matched;
    }
    std::string address = params.find("address")->second;
    if (!account_apiserver_traits<DB>::is_address_valid(address)) {
      LOG(WARNING)
          << "params value invalid, see nebulas address "
             "design, https://github.com/nebulasio/wiki/blob/master/address.md";
      return err_code_params_value_invalid;
    }

    account_cache_t &cache = *m_cache_ptr;
    std::shared_ptr<neb::account_info_t> info;

    if (!cache.get(address, info)) {
      LOG(INFO) << "account cache missed, reading from db";
      set_account_cache(cache, address);

      if (!cache.get(address, info)) {
        LOG(INFO) << "account db missed";
        return err_code_query_response_empty;
      }
    }
    return m_ac_ptr->account_info_to_string(*info);
  }

protected:
  void set_account_cache(account_cache_t &cache, const std::string &address) {
    std::vector<neb::account_info_t> rs =
        m_ac_ptr->read_account_by_address(address);
    LOG(INFO) << "read account by address, size: " << rs.size();
    for (auto it = rs.begin(); it != rs.end(); it++) {
      std::string address = it->template get<::neb::address>();
      cache.set(address, std::make_shared<neb::account_info_t>(*it));
    }
    return;
  }

protected:
  std::unique_ptr<account_cache_t> m_cache_ptr;
  account_db_ptr_t m_ac_ptr;
};

