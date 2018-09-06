#include "transaction_apiserver.h"
#include "err/err_def.h"

transaction_apiserver::transaction_apiserver(const std::string &appname)
    : apiserver(appname) {
  m_height_transaction_cache_ptr = std::unique_ptr<height_transaction_cache_t>(
      new height_transaction_cache_t());

  m_address_transaction_cache_ptr =
      std::unique_ptr<address_transaction_cache_t>(
          new address_transaction_cache_t());

  m_tx_ptr =
      std::make_shared<nebulas_transaction_db_t>(nebulas_transaction_db_t(
          std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
          std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB")));
}

void transaction_apiserver::set_height_transaction_cache(
    height_transaction_cache_t &cache, neb::block_height_t height) {

  int32_t read_ahead_height = 3600 / 15;
  std::vector<neb::transaction_info_t> txs =
      m_tx_ptr->read_success_and_failed_transaction_from_db_with_block_duration(
          height, height + read_ahead_height);
  LOG(INFO) << "read ahead transaction size: " << txs.size();
  std::unordered_map<neb::block_height_t, std::vector<neb::transaction_info_t>>
      height_and_txs;

  for (auto it = txs.begin(); it != txs.end(); it++) {
    neb::block_height_t height = it->template get<::neb::height>();
    height_and_txs[height].push_back(*it);
  }
  LOG(INFO) << "height and transaction done";

  for (neb::block_height_t h = height; h <= height + read_ahead_height; h++) {
    auto it = height_and_txs.find(h);
    LOG(INFO) << h;
    if (it != height_and_txs.end()) {
      cache.set(h, std::make_shared<std::vector<neb::transaction_info_t>>(
                       it->second));
    } else {
      cache.set(h, std::make_shared<std::vector<neb::transaction_info_t>>());
    }
  }
}

std::string transaction_apiserver::on_api_height_transaction(
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
  if (!neb::is_number(s_start_block) || !neb::is_number(s_end_block)) {
    LOG(WARNING) << "params value invalid, start_block/end_block value "
                    "contains unexpected character";
    return err_code_params_value_invalid;
  }

  neb::block_height_t start_block = std::stoi(s_start_block);
  neb::block_height_t end_block = std::stoi(s_end_block);

  height_transaction_cache_t &cache = *m_height_transaction_cache_ptr;
  std::shared_ptr<std::vector<neb::transaction_info_t>> txs;

  for (neb::block_height_t h = start_block; h <= end_block; h++) {
    std::shared_ptr<std::vector<neb::transaction_info_t>> rs;
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

  return m_tx_ptr->to_string(*txs);
}

void transaction_apiserver::set_address_transaction_cache(
    address_transaction_cache_t &cache, const std::string &address) {

  std::vector<neb::transaction_info_t> rs =
      m_tx_ptr->read_success_and_failed_transaction_from_db_with_address(
          address);
  LOG(INFO) << "read transaction by address, size: " << rs.size();
  cache.set(address, rs);
  return;
}

std::string transaction_apiserver::on_api_address_transaction(
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
  if (neb::nebulas::is_contract_address(address) < 0) {
    LOG(WARNING)
        << "params value invalid, see nebulas address "
           "design, https://github.com/nebulasio/wiki/blob/master/address.md";
    return err_code_params_value_invalid;
  }

  address_transaction_cache_t &cache = *m_address_transaction_cache_ptr;
  std::vector<neb::transaction_info_t> rs;
  if (!cache.get(address, rs)) {
    LOG(INFO) << "transaction cache missed, reading from db";
    set_address_transaction_cache(cache, address);

    if (!cache.get(address, rs)) {
      LOG(INFO) << "transaction db missed";
    }
  }
  LOG(INFO) << "transaction size: " << rs.size();

  return m_tx_ptr->to_string(rs);
}

bool transaction_apiserver::has_keys_in_params(
    const std::unordered_set<std::string> &keys,
    const std::unordered_map<std::string, std::string> &params) {
  for (auto it = params.begin(); it != params.end(); it++) {
    if (keys.find(it->first) == keys.end()) {
      return false;
    }
  }
  return true;
}

std::string transaction_apiserver::on_api_transaction(
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

