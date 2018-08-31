#pragma once

#include "apiserver.h"
#include "blockchain.h"
#include "cache/lru_cache.h"

typedef neb::lru_cache<neb::block_height_t,
                       std::vector<neb::transaction_info_t>>
    transaction_cache_t;
typedef neb::transaction_db<neb::nebulas_db> nebulas_transaction_db_t;
typedef std::shared_ptr<nebulas_transaction_db_t> nebulas_transaction_db_ptr_t;

class transaction_apiserver : public apiserver {
public:
  transaction_apiserver(const std::string &appname, size_t cache_size);

  std::string on_api_transaction(
      const std::unordered_map<std::string, std::string> &params);

private:
  void set_transaction_cache(transaction_cache_t &cache,
                             neb::block_height_t height);

protected:
  std::unique_ptr<transaction_cache_t> m_cache_ptr;
  nebulas_transaction_db_ptr_t m_tx_ptr;
};

