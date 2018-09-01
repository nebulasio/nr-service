#include "account_apiserver.h"

account_apiserver::account_apiserver(const std::string &appname,
                                     size_t cache_size)
    : apiserver(appname) {
  m_cache_ptr = std::make_unique<account_cache_t>(account_cache_t(cache_size));
  m_ac_ptr = std::make_shared<nebulas_account_db_t>(nebulas_account_db_t(
      STR(DB_URL), STR(DB_USER_NAME), STR(DB_PASSWORD), STR(NEBULAS_DB)));
}

std::string account_apiserver::on_api_account(
    const std::unordered_map<std::string, std::string> &params) {
  if (params.find("address") == params.end()) {
    return std::string();
  }
  std::string address = params.find("address")->second;
  if (neb::nebulas::is_contract_address(address) < 0) {
    return std::string();
  }

  account_cache_t &cache = *m_cache_ptr;
  neb::account_info_t info;
  if (!cache.get(address, info)) {
    LOG(INFO) << "account cache missed, reading from db";
    set_account_cache(cache, address);

    if (!cache.get(address, info)) {
      LOG(INFO) << "account db missed";
      return std::string();
    }
  }
  return m_ac_ptr->to_string(std::vector<neb::account_info_t>({info}));
}

void account_apiserver::set_account_cache(account_cache_t &cache,
                                          const std::string &address) {
  std::vector<neb::account_info_t> rs =
      m_ac_ptr->read_account_by_address(address);
  for (auto it = rs.begin(); it != rs.end(); it++) {
    std::string address = it->template get<::neb::address>();
    cache.set(address, *it);
  }
  return;
}
