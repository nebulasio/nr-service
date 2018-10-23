#include "analytics.h"
#include "blockchain.h"
#include "utils.h"

DEFINE_bool(loop, true, "run permenantly");
DEFINE_int32(start_ts, 0, "the first day end timestamp");
DEFINE_int32(end_ts, 1, "the last day end timestamp");

typedef std::shared_ptr<neb::transaction_db_interface> transaction_db_ptr_t;
typedef std::shared_ptr<neb::account_db_interface> account_db_ptr_t;
typedef std::shared_ptr<neb::nebulas::statistics_db_interface>
    nebulas_statistics_db_ptr_t;

typedef neb::nebulas::nebulas_transaction_db nebulas_transaction_db_t;
typedef neb::nebulas::nebulas_account_db nebulas_account_db_t;
typedef neb::nebulas::statistics_db nebulas_statistics_db_t;

std::vector<neb::transaction_info_t>
transaction_list_with_duration(const transaction_db_ptr_t db_ptr,
                               const std::string &start_ts,
                               const std::string &end_ts) {

  std::vector<neb::transaction_info_t> transactions =
      db_ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
          start_ts, end_ts);

  return transactions;
}

int32_t smart_contract_total_numbers(const account_db_ptr_t db_ptr,
                                     const std::string &end_ts) {
  int32_t ret = 0;
  std::vector<neb::account_info_t> accounts =
      db_ptr->read_account_from_db_with_create_ts_duration(std::to_string(0),
                                                           end_ts);
  for (auto it = accounts.begin(); it != accounts.end(); it++) {
    std::string type = it->template get<::neb::account_type>();
    if (type.compare("contract") == 0) {
      ret++;
    }
  }
  return ret;
}

typedef struct {
  size_t active_account_nums;
  size_t active_contract_nums;
} active_addr_t;

active_addr_t active_account_or_contract_numbers_with_duration(
    const transaction_db_ptr_t db_ptr, const std::string &start_ts,
    const std::string &end_ts) {

  std::vector<neb::transaction_info_t> transactions =
      transaction_list_with_duration(db_ptr, start_ts, end_ts);

  std::unordered_set<std::string> s_account;
  std::unordered_set<std::string> s_contract;

  for (auto it = transactions.begin(); it != transactions.end(); it++) {
    std::string from = it->template get<::neb::from>();
    std::string type_from = it->template get<::neb::type_from>();
    if (type_from.compare("normal") == 0) {
      s_account.insert(from);
    } else if (type_from.compare("contract") == 0) {
      s_contract.insert(from);
    }

    std::string to = it->template get<::neb::to>();
    std::string type_to = it->template get<::neb::type_to>();
    if (type_to.compare("normal") == 0) {
      s_account.insert(to);
    } else if (type_to.compare("contract") == 0) {
      s_contract.insert(to);
    }
  }
  return active_addr_t{s_account.size(), s_contract.size()};
}

void time_wait(time_t time_sec) {
  boost::asio::io_service io;
  boost::asio::deadline_timer t(io, boost::posix_time::seconds(time_sec));
  t.wait();
}

int main(int argc, char *argv[]) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  bool loop = FLAGS_loop;
  int32_t start_ts = FLAGS_start_ts;
  int32_t end_ts = FLAGS_end_ts;

  transaction_db_ptr_t tx_ptr = std::make_shared<nebulas_transaction_db_t>(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));

  account_db_ptr_t ac_ptr = std::make_shared<nebulas_account_db_t>(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));

  nebulas_statistics_db_ptr_t st_ptr =
      std::make_shared<nebulas_statistics_db_t>(
          std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
          std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));

  time_t seconds_of_day = 24 * 60 * 60;

  if (loop) {
    while (true) {
      time_t end_time = neb::time_utils::get_universal_timestamp();
      LOG(INFO) << end_time % seconds_of_day;
      if (end_time % seconds_of_day < 60) {
        time_t start_time = end_time - seconds_of_day;
        // using start time as date
        std::string date = neb::time_utils::time_t_to_date(start_time);

        std::string start_ts = std::to_string(start_time);
        std::string end_ts = std::to_string(end_time);

        int32_t contract_nums = smart_contract_total_numbers(ac_ptr, end_ts);
        active_addr_t active_nums =
            active_account_or_contract_numbers_with_duration(tx_ptr, start_ts,
                                                             end_ts);
        neb::nebulas::statistics_info_t info;
        info.template set<::neb::nebulas::date, ::neb::nebulas::contract_nums,
                          ::neb::nebulas::active_contract_nums,
                          ::neb::nebulas::active_account_nums>(
            date, contract_nums, active_nums.active_contract_nums,
            active_nums.active_account_nums);
        st_ptr->insert_statistics(info);

        LOG(INFO) << "date: " << date << ',' << contract_nums << ','
                  << active_nums.active_contract_nums << ','
                  << active_nums.active_account_nums;
      }
      time_wait(60);
    }
  }

  for (time_t ts = start_ts; ts < end_ts; ts += seconds_of_day) {
    time_t s = ts;
    time_t e = ts + seconds_of_day;
    std::string date = neb::time_utils::time_t_to_date(s);

    int32_t contract_nums =
        smart_contract_total_numbers(ac_ptr, std::to_string(e));
    active_addr_t active_nums =
        active_account_or_contract_numbers_with_duration(
            tx_ptr, std::to_string(s), std::to_string(e));

    neb::nebulas::statistics_info_t info;
    info.template set<::neb::nebulas::date, ::neb::nebulas::contract_nums,
                      ::neb::nebulas::active_contract_nums,
                      ::neb::nebulas::active_account_nums>(
        date, contract_nums, active_nums.active_contract_nums,
        active_nums.active_account_nums);
    st_ptr->insert_statistics(info);

    LOG(INFO) << "date: " << date << ',' << contract_nums << ','
              << active_nums.active_contract_nums << ','
              << active_nums.active_account_nums;
  }
  return 0;
}
