#include "blockchain/eth/eth_account_db.h"
#include "blockchain/eth/eth_api.h"

namespace neb {
namespace eth {

eth_account_db::eth_account_db() {}
eth_account_db::eth_account_db(const std::string &url,
                               const std::string &usrname,
                               const std::string &passwd,
                               const std::string &dbname)
    : account_db<eth_db>(url, usrname, passwd, dbname) {}

void eth_account_db::insert_account_to_db(block_height_t start_block,
                                          block_height_t end_block) {
  std::vector<transaction_info_t> txs =
      m_tdb_ptr
          ->read_success_and_failed_transaction_from_db_with_block_duration(
              start_block, end_block);
  LOG(INFO) << "transaction size: " << txs.size();

  std::unordered_map<account_address_t, account_info_t> addr_and_info;

  for (auto tx : txs) {
    set_accounts(tx, addr_and_info);
  }
  LOG(INFO) << "to update size: " << addr_and_info.size();

  std::vector<std::pair<account_address_t, account_info_t>> sorted_info =
      sort_account_info_by_height(addr_and_info);

  uint32_t cnt = 1;
  for (auto it = sorted_info.begin(); it != sorted_info.end(); it++) {
    std::string address = it->first;
    block_height_t height = it->second.template get<::neb::height>();
    LOG(INFO) << "address: " << address << ", height: " << height
              << ", cnt: " << cnt
              << ", left to update: " << sorted_info.size() - cnt;
    cnt++;

    std::string balance = get_address_balance(
        address, string_utils::to_hex(std::to_string(height)));

    account_info_t &info = it->second;
    info.template set<::neb::balance>(string_utils::to_dec(balance));

    insert_account(info);
  }
  LOG(INFO) << "rows: " << cnt;
}

std::vector<std::pair<account_address_t, account_info_t>>
eth_account_db::sort_account_info_by_height(
    const std::unordered_map<account_address_t, account_info_t>
        &addr_and_account_info) {

  std::vector<std::pair<account_address_t, account_info_t>> sorted_info;
  for (auto it = addr_and_account_info.begin();
       it != addr_and_account_info.end(); it++) {
    std::string addr = it->first;
    account_info_t info = it->second;
    sorted_info.push_back(std::make_pair(addr, info));
  }

  auto cmp =
      [](const std::pair<account_address_t, account_info_t> &info1,
         const std::pair<account_address_t, account_info_t> &info2) -> bool {
    return info1.second.template get<::neb::height>() <
           info2.second.template get<::neb::height>();
  };

  sort(sorted_info.begin(), sorted_info.end(), cmp);
  return sorted_info;
}

void eth_account_db::set_accounts(
    const transaction_info_t &info,
    std::unordered_map<account_address_t, account_info_t> &addr_and_info) {

  const transaction_info_t &tx = info;
  block_height_t height = tx.template get<::neb::height>();

  std::string from = tx.template get<::neb::from>();
  std::string type_from = tx.template get<::neb::type_from>();

  if (from.compare("none") != 0) {
    auto it = addr_and_info.find(from);
    if (it == addr_and_info.end()) {
      account_info_t info_from;
      info_from
          .template set<::neb::address, ::neb::account_type, ::neb::height>(
              from, type_from, height);
      addr_and_info.insert(std::make_pair(from, info_from));
    } else {
      account_info_t &info_from = it->second;
      if (info_from.template get<::neb::height>() < height) {
        info_from.template set<::neb::height>(height);
      }
    }
  }

  std::string to = tx.template get<::neb::to>();
  std::string type_to = tx.template get<::neb::type_to>();

  if (to.compare("none") != 0) {
    auto it = addr_and_info.find(to);
    if (it == addr_and_info.end()) {
      account_info_t info_to;
      info_to.template set<::neb::address, ::neb::account_type, ::neb::height>(
          to, type_to, height);
      addr_and_info.insert(std::make_pair(to, info_to));
    } else {
      account_info_t &info_to = it->second;
      if (info_to.template get<::neb::height>() < height) {
        info_to.template set<::neb::height>(height);
      }
    }
  }

  std::string contract_address = tx.template get<::neb::contract_address>();
  if (contract_address.empty()) {
    return;
  }
  auto it = addr_and_info.find(contract_address);
  if (it == addr_and_info.end()) {
    account_info_t info_contract;
    info_contract
        .template set<::neb::address, ::neb::account_type, ::neb::height>(
            contract_address, "contract", height);
  } else {
    account_info_t &info_contract = it->second;
    if (info_contract.template get<::neb::height>() < height) {
      info_contract.template set<::neb::height>(height);
    }
  }
}

void eth_account_db::insert_account(const account_info_t &info) {

  std::string address = info.template get<::neb::address>();
  std::string account_type = info.template get<::neb::account_type>();
  block_height_t height = info.template get<::neb::height>();
  std::string balance = info.template get<::neb::balance>();

  const std::string aql = boost::str(
      boost::format(
          "upsert {_key:'%1%'} insert {_key:'%1%', address:'%1%', type:'%2%', "
          "height:%3%, balance:'%4%'} update {address:'%1%', type:'%2%',  "
          "height:%3%, balance:'%4%'} in account") %
      address % account_type % height % balance);
  aql_query(aql);
}

} // namespace eth
} // namespace neb
