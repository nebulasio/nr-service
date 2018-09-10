#include "blockchain/nebulas/nebulas_account_db.h"
#include "blockchain/nebulas/nebulas_api.h"
#include <boost/format.hpp>

namespace neb {
namespace nebulas {

nebulas_account_db::nebulas_account_db() {}
nebulas_account_db::nebulas_account_db(const std::string &url,
                                       const std::string &usrname,
                                       const std::string &passwd,
                                       const std::string &dbname)
    : account_db<nebulas_db>(url, usrname, passwd, dbname) {}

std::vector<std::pair<account_address_t, account_info_t>>
nebulas_account_db::sort_update_info_by_height(
    const std::unordered_map<account_address_t, account_info_t>
        &to_update_info) {

  std::vector<std::pair<account_address_t, account_info_t>> sorted_update_info;
  for (auto it = to_update_info.begin(); it != to_update_info.end(); it++) {
    std::string addr = it->first;
    account_info_t info = it->second;
    sorted_update_info.push_back(std::make_pair(addr, info));
  }

  auto cmp =
      [](const std::pair<account_address_t, account_info_t> &info1,
         const std::pair<account_address_t, account_info_t> &info2) -> bool {
    return info1.second.template get<::neb::height>() <
           info2.second.template get<::neb::height>();
  };

  sort(sorted_update_info.begin(), sorted_update_info.end(), cmp);
  return sorted_update_info;
}

void nebulas_account_db::set_coinbase_account() {
  // coinbase address
  std::string address = "n1dZZnqKGEkb1LHYsZRei1CH6DunTio1j1q";
  block_height_t height = get_max_height_from_db();
  std::pair<std::string, account_type_t> account_state =
      ::neb::nebulas::get_account_state(height, address);
  if (account_state.second == -1) {
    LOG(INFO) << "exit.";
    return;
  }
  std::string balance = account_state.first;
  std::string account_type = "normal";
  // created at block height 2
  std::string timestamp = "1522377345";

  account_info_t info;
  info.template set<::neb::address, ::neb::balance, ::neb::height,
                    ::neb::account_type, ::neb::create_at>(
      address, balance, height, account_type, timestamp);
  insert_document_to_collection(info);
}

block_height_t nebulas_account_db::get_max_height_from_db() {

  std::unique_ptr<::arangodb::fuerte::Response> resp_ptr =
      aql_query("for h in account sort h.height desc limit 1 return h.height");

  auto height_doc = resp_ptr->slices().front().get("result");
  if (height_doc.isNone() || height_doc.isEmptyArray()) {
    return 0;
  }
  return height_doc.at(0).getInt();
}

void nebulas_account_db::insert_document_to_collection(
    const account_info_t &info) {
  std::string address = info.template get<::neb::address>();
  std::string balance = info.template get<::neb::balance>();
  std::string account_type = info.template get<::neb::account_type>();
  std::string create_at = info.template get<::neb::create_at>();
  block_height_t height = info.template get<::neb::height>();

  const std::string aql = boost::str(
      boost::format("upsert {_key:'%1%'} "
                    "insert {_key:'%1%', address:'%1%', "
                    "balance:'%2%', type:'%3%', create_at:'%4%', height:%5%} "
                    "update {address:'%1%', balance:'%2%', type:'%3%', "
                    "create_at:'%4%', height:%5%} in account") %
      address % balance % account_type % create_at % height);
  aql_query(aql);
}

void nebulas_account_db::append_account_to_db() {
  block_height_t last_height = get_max_height_from_db();

  std::vector<transaction_info_t> txs =
      m_tdb_ptr
          ->read_success_and_failed_transaction_from_db_with_block_duration(
              last_height, std::numeric_limits<int64_t>::max());

  std::vector<account_info_t> account_info_list = read_account_from_db();
  std::unordered_map<account_address_t, account_info_t>
      address_and_account_info;
  for (auto it = account_info_list.begin(); it != account_info_list.end();
       it++) {
    address_and_account_info.insert(
        std::make_pair(it->template get<::neb::address>(), *it));
  }
  std::unordered_map<account_address_t, account_info_t> to_update_info;

  for (auto it = txs.begin(); it != txs.end(); it++) {
    std::string timestamp = it->template get<::neb::timestamp>();
    int64_t height = it->template get<::neb::height>();

    std::string from = it->template get<::neb::from>();
    account_info_t info_from;
    info_from.template set<::neb::address, ::neb::create_at, ::neb::height>(
        from, timestamp, height);

    if (to_update_info.find(from) == to_update_info.end()) {
      if (address_and_account_info.find(from) !=
          address_and_account_info.end()) {
        info_from.template set<::neb::create_at>(
            address_and_account_info[from].template get<::neb::create_at>());
      }
      to_update_info.insert(std::make_pair(from, info_from));
    } else {
      info_from.template set<::neb::create_at>(
          to_update_info[from].template get<::neb::create_at>());
      to_update_info[from] = info_from;
    }

    std::string to = it->template get<::neb::to>();
    account_info_t info_to;
    info_to.template set<::neb::address, ::neb::create_at, ::neb::height>(
        to, timestamp, height);

    if (to_update_info.find(to) == to_update_info.end()) {
      if (address_and_account_info.find(to) != address_and_account_info.end()) {
        info_to.template set<::neb::create_at>(
            address_and_account_info[to].template get<::neb::create_at>());
      }
      to_update_info.insert(std::make_pair(to, info_to));
    } else {
      info_to.template set<::neb::create_at>(
          to_update_info[to].template get<::neb::create_at>());
      to_update_info[to] = info_to;
    }

    std::string contract_address = it->template get<::neb::contract_address>();
    if (contract_address.empty()) {
      continue;
    }
    account_info_t info_contract;
    info_contract.template set<::neb::address, ::neb::create_at, ::neb::height>(
        contract_address, timestamp, height);

    if (to_update_info.find(contract_address) == to_update_info.end()) {
      if (address_and_account_info.find(contract_address) !=
          address_and_account_info.end()) {
        info_contract.template set<::neb::create_at>(
            address_and_account_info[contract_address]
                .template get<::neb::create_at>());
      }
      to_update_info.insert(std::make_pair(contract_address, info_contract));
    } else {
      info_contract.template set<::neb::create_at>(
          to_update_info[contract_address].template get<::neb::create_at>());
      to_update_info[contract_address] = info_contract;
    }
  }

  LOG(INFO) << "to update size: " << to_update_info.size();

  std::vector<std::pair<account_address_t, account_info_t>> sorted_update_info =
      sort_update_info_by_height(to_update_info);

  int cnt = 1;
  for (auto it = sorted_update_info.begin(); it != sorted_update_info.end();
       it++) {
    std::string address = it->first;
    int height = it->second.template get<::neb::height>();
    LOG(INFO) << "address: " << address << ", height: " << height
              << ", cnt: " << cnt++;

    int32_t ret_type = is_contract_address(address);
    if (ret_type == -1) {
      return;
    }
    std::string type = (ret_type == 0 ? "normal" : "contract");

    std::pair<std::string, account_type_t> account_state =
        get_account_state(height, address);
    if (account_state.second == -1) {
      LOG(INFO) << "exit.";
      return;
    }
    std::string balance = account_state.first;

    account_info_t &info = it->second;
    info.template set<::neb::balance, ::neb::account_type>(balance, type);

    insert_document_to_collection(info);
  }
  LOG(INFO) << "rows: " << cnt;
  set_coinbase_account();
}

} // end namespace nebulas
} // end namespace neb
