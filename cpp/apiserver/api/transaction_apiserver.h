#pragma once

#include "apiserver.h"
#include "blockchain.h"

typedef neb::lru_cache<neb::block_height_t,
                       std::shared_ptr<std::vector<neb::transaction_info_t>>>
    height_transaction_cache_t;
typedef neb::lru_cache<std::string,
                       std::shared_ptr<std::vector<neb::transaction_info_t>>>
    address_transaction_cache_t;
typedef neb::transaction_db<neb::nebulas_db> nebulas_transaction_db_t;
typedef std::shared_ptr<nebulas_transaction_db_t> nebulas_transaction_db_ptr_t;

class transaction_apiserver : public apiserver {
public:
  transaction_apiserver(const std::string &appname);

  std::string on_api_transaction(
      const std::unordered_map<std::string, std::string> &params);

private:
  bool has_keys_in_params(
      const std::unordered_set<std::string> &keys,
      const std::unordered_map<std::string, std::string> &params);

  std::string on_api_height_transaction(
      const std::unordered_map<std::string, std::string> &params);

  std::string on_api_address_transaction(
      const std::unordered_map<std::string, std::string> &params);

  void set_height_transaction_cache(height_transaction_cache_t &cache,
                                    neb::block_height_t height);

  void set_address_transaction_cache(address_transaction_cache_t &cache,
                                     const std::string &address);

protected:
  std::unique_ptr<height_transaction_cache_t> m_height_transaction_cache_ptr;
  std::unique_ptr<address_transaction_cache_t> m_address_transaction_cache_ptr;
  nebulas_transaction_db_ptr_t m_tx_ptr;
};

