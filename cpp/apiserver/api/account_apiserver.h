#pragma once

#include "apiserver.h"
#include "blockchain.h"

typedef neb::lru_cache<std::string, std::shared_ptr<neb::account_info_t>>
    account_cache_t;
typedef neb::account_db<neb::nebulas_db> nebulas_account_db_t;
typedef std::shared_ptr<nebulas_account_db_t> nebulas_account_db_ptr_t;

class account_apiserver : public apiserver {
public:
  account_apiserver(const std::string &appname);

  std::string
  on_api_account(const std::unordered_map<std::string, std::string> &params);

private:
  void set_account_cache(account_cache_t &cache, const std::string &address);

protected:
  std::unique_ptr<account_cache_t> m_cache_ptr;
  nebulas_account_db_ptr_t m_ac_ptr;
};

