#include "account_apiserver.h"
#include "err/err_def.h"
#include <memory>

account_apiserver::account_apiserver(const std::string &appname)
    : apiserver(appname) {
  m_cache_ptr = std::unique_ptr<account_cache_t>(new account_cache_t());
  m_ac_ptr = std::make_shared<nebulas_account_db_t>(nebulas_account_db_t(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB")));
}

std::string account_apiserver::on_api_account(
    const std::unordered_map<std::string, std::string> &params) {

  LOG(INFO) << "on api account, request params";
  for (auto it = params.begin(); it != params.end(); it++) {
    LOG(INFO) << it->first << ',' << it->second;
  }

  if (params.find("address") == params.end()) {
    LOG(WARNING) << "params not matched, no param \'address\'";
    return err_code_params_not_matched;
  }
  std::string address = params.find("address")->second;
  if (neb::nebulas::is_contract_address(address) < 0) {
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
  return nebulas_account_db_t::account_info_to_string(*info);
}

void account_apiserver::set_account_cache(account_cache_t &cache,
                                          const std::string &address) {
  std::vector<neb::account_info_t> rs =
      m_ac_ptr->read_account_by_address(address);
  LOG(INFO) << "read account by address, size: " << rs.size();
  for (auto it = rs.begin(); it != rs.end(); it++) {
    std::string address = it->template get<::neb::address>();
    cache.set(address, std::make_shared<neb::account_info_t>(*it));
  }
  return;
}
