#include "transaction_apiserver.h"

transaction_apiserver::transaction_apiserver(const std::string &appname,
                                             size_t cache_size)
    : apiserver(appname) {
  m_cache_ptr =
      std::make_unique<transaction_cache_t>(transaction_cache_t(cache_size));
  m_tx_ptr =
      std::make_shared<nebulas_transaction_db_t>(nebulas_transaction_db_t(
          STR(DB_URL), STR(DB_USER_NAME), STR(DB_PASSWORD), STR(NEBULAS_DB)));
}

void transaction_apiserver::set_transaction_cache(transaction_cache_t &cache,
                                                  neb::block_height_t height) {
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

std::string transaction_apiserver::on_api_transaction(
    const std::unordered_map<std::string, std::string> &params) {

  if (params.find("start_block") == params.end() ||
      params.find("end_block") == params.end()) {
    return std::string();
  }

  std::string s_start_block = params.find("start_block")->second;
  std::string s_end_block = params.find("end_block")->second;
  if (!neb::is_number(s_start_block) || !neb::is_number(s_end_block)) {
    return std::string();
  }

  neb::block_height_t start_block = std::stoi(s_start_block);
  neb::block_height_t end_block = std::stoi(s_end_block);

  transaction_cache_t &cache = *m_cache_ptr;
  std::vector<neb::transaction_info_t> txs;

  for (neb::block_height_t h = start_block; h <= end_block; h++) {
    std::vector<neb::transaction_info_t> rs;
    if (!cache.get(h, rs)) {
      LOG(INFO) << "cache missed, reading from db";
      set_transaction_cache(cache, h);

      if (!cache.get(h, rs)) {
        LOG(INFO) << "db missed";
      }
    }
    txs.insert(txs.end(), rs.begin(), rs.end());
  }
  LOG(INFO) << "transaction size: " << txs.size();
  return m_tx_ptr->to_string(txs);
}
