#include "apiserver.h"
#include "utils.h"
#include <iostream>

apiserver::apiserver(const std::string &appname, size_t cache_size)
    : m_appname(appname) {
  m_cache_ptr = std::make_shared<cache_t>(cache_t(cache_size));
  m_tx_ptr =
      std::make_shared<nebulas_transaction_db_t>(nebulas_transaction_db_t(
          STR(DB_URL), STR(DB_USER_NAME), STR(DB_PASSWORD), STR(NEBULAS_DB)));
}

void apiserver::on_api_example(
    const std::unordered_map<std::string, std::string> &params) {
  std::cout << "on_api_example" << std::endl;
}

void apiserver::set_cache(cache_t &cache, neb::block_height_t height) {

  int32_t read_ahead_height = 3600 / 15;
  std::vector<neb::transaction_info_t> txs =
      m_tx_ptr->read_success_and_failed_transaction_from_db_with_duration(
          height, height + read_ahead_height);
  LOG(INFO) << "read ahead transaction size: " << txs.size();
  std::unordered_map<neb::block_height_t, std::vector<neb::transaction_info_t>>
      height_and_txs;

  for (auto it = txs.begin(); it != txs.end(); it++) {
    neb::block_height_t height = it->template get<::neb::height>();
    height_and_txs[height].push_back(*it);
  }

  for (neb::block_height_t h = height; h <= height + read_ahead_height; h++) {
    auto it = height_and_txs.find(h);
    if (it != height_and_txs.end()) {
      cache.set(h, it->second);
    } else {
      cache.set(h, std::vector<neb::transaction_info_t>());
    }
  }
}

std::string apiserver::on_api_transaction(int64_t start_block,
                                          int64_t end_block) {
  cache_t &cache = *m_cache_ptr;
  std::vector<neb::transaction_info_t> txs;

  for (neb::block_height_t h = start_block; h <= end_block; h++) {
    std::vector<neb::transaction_info_t> rs;
    if (!cache.get(h, rs)) {
      LOG(INFO) << "cache missed, reading from db";
      set_cache(cache, h);

      // if (!cache.get(h, rs)) {
      // LOG(INFO) << "db missed";
      //}
    }
    txs.insert(txs.end(), rs.begin(), rs.end());
  }
  LOG(INFO) << "transaction size: " << txs.size();
  return m_tx_ptr->to_string(txs);
}
