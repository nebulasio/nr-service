#pragma once

#include "apiserver.h"
#include "blockchain.h"
#include "cache/lru_cache.h"

typedef neb::lru_cache<std::string, neb::account_info_t> account_cache_t;
typedef neb::account_db<neb::nebulas_db> nebulas_account_db_t;
typedef std::shared_ptr<nebulas_account_db_t> nebulas_account_db_ptr_t;

class account_apiserver : public apiserver {
public:
  account_apiserver(const std::string &appname, size_t cache_size);

  std::string
  on_api_account(const std::unordered_map<std::string, std::string> &params);

private:
  void set_account_cache(account_cache_t &cache, const std::string &address);

protected:
  std::unique_ptr<account_cache_t> m_cache_ptr;
  nebulas_account_db_ptr_t m_ac_ptr;
};

