#include "blockchain.h"
#include "nr.h"
#include "utils.h"

DEFINE_int32(start_ts, 0, "the first day end timestamp");
DEFINE_int32(end_ts, 1, "the last day end timestamp");

typedef neb::transaction_db<neb::nebulas_db> nebulas_transaction_db_t;
typedef std::shared_ptr<neb::transaction_db_interface> transaction_db_ptr_t;

typedef neb::balance_db<neb::nebulas_db> nebulas_balance_db_t;
typedef std::shared_ptr<neb::balance_db_interface> balance_db_ptr_t;

void write_date_balance(transaction_db_ptr_t tdb_ptr, balance_db_ptr_t bdb_ptr,
                        const std::string &date,
                        neb::block_height_t start_block,
                        neb::block_height_t end_block) {
  auto account_inter_txs =
      tdb_ptr->read_inter_transaction_from_db_with_duration(start_block,
                                                            end_block);
  std::unordered_map<std::string, std::string> addr_and_type;
  for (auto &tx : account_inter_txs) {
    auto from = tx.template get<::neb::from>();
    auto type_from = tx.template get<::neb::type_from>();
    addr_and_type.insert(std::make_pair(from, type_from));

    auto to = tx.template get<::neb::to>();
    auto type_to = tx.template get<::neb::type_to>();
    addr_and_type.insert(std::make_pair(to, type_to));
  }

  std::vector<neb::balance_info_t> rs;
  for (auto &addr : addr_and_type) {
    auto account_state =
        neb::nebulas::get_account_state(start_block, addr.first);

    std::string balance = account_state.first;
    std::string type = addr.second;

    neb::balance_info_t info;
    info.template set<::neb::date, ::neb::address, ::neb::balance,
                      ::neb::account_type>(date, addr.first, balance, type);
    rs.push_back(info);
  }

  LOG(INFO) << "insert balance db begin...";
  bdb_ptr->insert_date_balances(rs);
  LOG(INFO) << "insert balance db done";
}

void write_to_balance_db(transaction_db_ptr_t tdb_ptr, balance_db_ptr_t bdb_ptr,
                         time_t end_ts) {

  time_t seconds_of_day = 24 * 60 * 60;
  time_t seconds_of_ten_minute = 10 * 60;

  std::vector<neb::transaction_info_t> txs_in_end_last_minute =
      tdb_ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
          std::to_string(end_ts - seconds_of_ten_minute),
          std::to_string(end_ts));
  if (txs_in_end_last_minute.empty()) {
    LOG(INFO) << "no transactions in end timestamp of last minute";
    return;
  }
  neb::block_height_t end_block =
      txs_in_end_last_minute.back().template get<::neb::height>();

  time_t start_ts = end_ts - seconds_of_day;
  std::vector<neb::transaction_info_t> txs_in_start_last_minute =
      tdb_ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
          std::to_string(start_ts - seconds_of_ten_minute),
          std::to_string(start_ts));
  if (txs_in_start_last_minute.empty()) {
    LOG(INFO) << "no transactions in start timestamp of last minute";
    return;
  }
  neb::block_height_t start_block =
      txs_in_start_last_minute.back().template get<::neb::height>();

  std::string date = neb::time_t_to_date(start_ts);
  LOG(INFO) << date;
  write_date_balance(tdb_ptr, bdb_ptr, date, start_block, end_block);
}

int main(int argc, char *argv[]) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  int32_t start_ts = FLAGS_start_ts;
  int32_t end_ts = FLAGS_end_ts;

  nebulas_transaction_db_t tdb(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  transaction_db_ptr_t tdb_ptr =
      std::make_shared<nebulas_transaction_db_t>(tdb);

  nebulas_balance_db_t bdb(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                           std::getenv("DB_PASSWORD"),
                           std::getenv("NEBULAS_DB"));
  balance_db_ptr_t bdb_ptr = std::make_shared<nebulas_balance_db_t>(bdb);

  time_t seconds_of_day = 24 * 60 * 60;

  for (time_t ts = start_ts; ts < end_ts; ts += seconds_of_day) {
    write_to_balance_db(tdb_ptr, bdb_ptr, ts);
  }
  return 0;
}
