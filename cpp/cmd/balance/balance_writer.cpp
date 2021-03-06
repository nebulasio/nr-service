#include "blockchain.h"
#include "nr.h"
#include "utils.h"

DEFINE_string(chain, "nebulas", "chain name, nebulas or eth");
DEFINE_int32(days, 1, "balance db days interval");
DEFINE_int32(start_ts, 0, "the first day end timestamp");
DEFINE_int32(end_ts, 1, "the last day end timestamp");
DEFINE_int32(thread_nums, 1, "parallel thread numbers");
DEFINE_bool(auto_start, true, "using auto start timestamp");

typedef neb::transaction_db_interface transaction_db_t;
typedef std::shared_ptr<transaction_db_t> tdb_ptr_t;
typedef neb::account_db_interface account_db_t;
typedef std::shared_ptr<account_db_t> adb_ptr_t;
typedef neb::balance_db_interface balance_db_t;
typedef std::shared_ptr<balance_db_t> bdb_ptr_t;

typedef neb::transaction_db<neb::nebulas_db> nebulas_transaction_db_t;
typedef neb::account_db<neb::nebulas_db> nebulas_account_db_t;
typedef neb::balance_db<neb::nebulas_db> nebulas_balance_db_t;

typedef neb::transaction_db<neb::eth_db> eth_transaction_db_t;
typedef neb::account_db<neb::eth_db> eth_account_db_t;
typedef neb::balance_db<neb::eth_db> eth_balance_db_t;

std::string balance_db_name = "balance_";
time_t seconds_of_day = 24 * 60 * 60;
time_t seconds_interval = 0;

void divide_address_set(
    const std::unordered_map<std::string, std::string> &addr_and_type,
    size_t parallel,
    std::vector<std::unordered_map<std::string, std::string>> &addr_type_list) {

  size_t interval = addr_and_type.size() / parallel;
  std::unordered_map<std::string, std::string> tmp;

  for (auto &addr : addr_and_type) {
    tmp.insert(std::make_pair(addr.first, addr.second));
    if (tmp.size() == interval) {
      addr_type_list.push_back(tmp);
      tmp.clear();
    }
  }
  if (!addr_type_list.empty()) {
    auto &last = addr_type_list.back();
    for (auto &it_tmp : tmp) {
      last.insert(std::make_pair(it_tmp.first, it_tmp.second));
    }
  }
}

void get_address_parallel(
    const std::vector<std::unordered_map<std::string, std::string>>
        &addr_type_list,
    adb_ptr_t adb_ptr, neb::block_height_t block_height,
    std::unordered_map<std::string, std::string> &addr_and_balance) {

  std::vector<std::thread> tv;
  std::mutex lock;

  for (size_t i = 0; i < addr_type_list.size(); i++) {
    std::thread t([&, i]() {
      std::unordered_map<std::string, std::string> tmp;
      for (auto &addr : addr_type_list[i]) {
        std::string balance = adb_ptr->get_address_balance(
            addr.first, std::to_string(block_height));
        tmp.insert(std::make_pair(addr.first, balance));
      }
      std::lock_guard<std::mutex> lg(lock);
      addr_and_balance.insert(tmp.begin(), tmp.end());
    });
    tv.push_back(std::move(t));
  }

  for (auto &t : tv) {
    t.join();
  }
}

void write_date_balance(tdb_ptr_t tdb_ptr, adb_ptr_t adb_ptr, bdb_ptr_t bdb_ptr,
                        const std::string &date,
                        neb::block_height_t start_block,
                        neb::block_height_t end_block, size_t parallel) {
  auto it_account_inter_txs =
      tdb_ptr->read_succ_and_failed_inter_transaction_from_db_with_duration(
          start_block, end_block);
  auto account_inter_txs = *it_account_inter_txs;
  LOG(INFO) << "account to account size: " << account_inter_txs.size();
  std::unordered_map<std::string, std::string> addr_and_type;
  for (auto &tx : account_inter_txs) {
    auto from = tx.template get<::neb::from>();
    auto type_from = tx.template get<::neb::type_from>();
    addr_and_type.insert(std::make_pair(from, type_from));

    auto to = tx.template get<::neb::to>();
    auto type_to = tx.template get<::neb::type_to>();
    addr_and_type.insert(std::make_pair(to, type_to));
  }
  LOG(INFO) << "account set size: " << addr_and_type.size();

  std::vector<std::unordered_map<std::string, std::string>> addr_type_list;
  divide_address_set(addr_and_type, parallel, addr_type_list);

  std::unordered_map<std::string, std::string> addr_and_balance;
  get_address_parallel(addr_type_list, adb_ptr, start_block, addr_and_balance);

  std::vector<neb::balance_info_t> rs;
  for (auto &addr : addr_and_balance) {
    // std::string balance =
    // adb_ptr->get_address_balance(addr.first, std::to_string(start_block));
    std::string balance = addr.second;

    neb::balance_info_t info;
    info.template set<::neb::date, ::neb::address, ::neb::balance,
                      ::neb::account_type>(date, addr.first, balance, "normal");
    rs.push_back(info);
  }

  LOG(INFO) << "insert balance db begin...";
  bdb_ptr->insert_date_balances(rs, balance_db_name);
  LOG(INFO) << "insert balance db done";
}

void write_to_balance_db(tdb_ptr_t tdb_ptr, adb_ptr_t adb_ptr,
                         bdb_ptr_t bdb_ptr, time_t end_ts, size_t parallel) {

  auto it_txs_in_end_last_day =
      tdb_ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
          std::to_string(end_ts - seconds_of_day), std::to_string(end_ts));
  auto txs_in_end_last_day = *it_txs_in_end_last_day;
  if (txs_in_end_last_day.empty()) {
    LOG(INFO) << "no transactions in end timestamp of last day";
    return;
  }
  neb::block_height_t end_block =
      txs_in_end_last_day.back().template get<::neb::height>();

  time_t start_ts = end_ts - seconds_interval;
  auto it_txs_in_start_last_day =
      tdb_ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
          std::to_string(start_ts - seconds_of_day), std::to_string(start_ts));
  auto txs_in_start_last_day = *it_txs_in_start_last_day;
  if (txs_in_start_last_day.empty()) {
    LOG(INFO) << "no transactions in start timestamp of last day";
    return;
  }
  neb::block_height_t start_block =
      txs_in_start_last_day.back().template get<::neb::height>();

  std::string start_date = neb::time_utils::time_t_to_date(start_ts);
  std::string end_date = neb::time_utils::time_t_to_date(end_ts);
  std::string date_range = start_date + '-' + end_date;
  LOG(INFO) << date_range << ',' << start_block << ',' << end_block;
  write_date_balance(tdb_ptr, adb_ptr, bdb_ptr, date_range, start_block,
                     end_block, parallel);
}

struct db_ptr_set_t {
  tdb_ptr_t tdb_ptr;
  adb_ptr_t adb_ptr;
  bdb_ptr_t bdb_ptr;
};

db_ptr_set_t get_db_ptr_set(const std::string &chain) {
  std::string db_url = std::getenv("DB_URL");
  std::string db_usrname = std::getenv("DB_USER_NAME");
  std::string db_passwd = std::getenv("DB_PASSWORD");

  if (chain.compare("nebulas") != 0 && chain.compare("eth") != 0) {
    LOG(INFO) << "invalid chain, type 'nebulas' or 'eth'";
    exit(-1);
  }

  if (chain.compare("nebulas") == 0) {
    std::string db_name = std::getenv("NEBULAS_DB");

    nebulas_transaction_db_t tdb(db_url, db_usrname, db_passwd, db_name);
    nebulas_account_db_t adb(db_url, db_usrname, db_passwd, db_name);
    nebulas_balance_db_t bdb(db_url, db_usrname, db_passwd, db_name);

    tdb_ptr_t tdb_ptr = std::make_shared<nebulas_transaction_db_t>(tdb);
    adb_ptr_t adb_ptr = std::make_shared<nebulas_account_db_t>(adb);
    bdb_ptr_t bdb_ptr = std::make_shared<nebulas_balance_db_t>(bdb);

    return db_ptr_set_t{tdb_ptr, adb_ptr, bdb_ptr};
  }

  std::string db_name = std::getenv("ETH_DB");

  eth_transaction_db_t tdb(db_url, db_usrname, db_passwd, db_name);
  eth_account_db_t adb(db_url, db_usrname, db_passwd, db_name);
  eth_balance_db_t bdb(db_url, db_usrname, db_passwd, db_name);

  tdb_ptr_t tdb_ptr = std::make_shared<eth_transaction_db_t>(tdb);
  adb_ptr_t adb_ptr = std::make_shared<eth_account_db_t>(adb);
  bdb_ptr_t bdb_ptr = std::make_shared<eth_balance_db_t>(bdb);

  return db_ptr_set_t{tdb_ptr, adb_ptr, bdb_ptr};
}

time_t get_balance_db_start_ts(const std::string &chain,
                               const std::string &balance_db_name) {

  assert(chain.compare("nebulas") == 0 || chain.compare("eth") == 0);
  std::unique_ptr<::arangodb::fuerte::Response> resp_ptr;

  std::string aql = boost::str(
      boost::format(
          "for item in %1% sort item.date desc limit 1 return item.date") %
      balance_db_name);

  if (chain.compare("nebulas") == 0) {
    std::shared_ptr<nebulas_balance_db_t> ptr =
        std::make_shared<nebulas_balance_db_t>(
            std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
            std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
    resp_ptr = ptr->aql_query(aql, 1);
  } else {
    std::shared_ptr<eth_balance_db_t> ptr = std::make_shared<eth_balance_db_t>(
        std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
        std::getenv("DB_PASSWORD"), std::getenv("ETH_DB"));
    resp_ptr = ptr->aql_query(aql, 1);
  }

  auto date_doc = resp_ptr->slices().front().get("result");
  if (date_doc.isNone() || date_doc.isEmptyArray()) {
    return 0;
  }
  std::string date_range = date_doc.at(0).copyString();
  LOG(INFO) << "date range " << date_range;
  std::string date = date_range.substr(9);
  std::string year = date.substr(0, 4);
  std::string month = date.substr(4, 2);
  std::string day = date.substr(6);

  std::string pt_str =
      boost::str(boost::format("%1%-%2%-%3% 00:00:00") % year % month % day);
  LOG(INFO) << pt_str;
  boost::posix_time::ptime pt = boost::posix_time::time_from_string(pt_str);
  return boost::posix_time::to_time_t(pt);
}

int main(int argc, char *argv[]) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::string chain = FLAGS_chain;
  int32_t days = FLAGS_days;
  balance_db_name = balance_db_name + std::to_string(days);
  seconds_interval = seconds_of_day * days;

  int32_t start_ts = FLAGS_start_ts;
  int32_t end_ts = FLAGS_end_ts;
  size_t thread_nums = FLAGS_thread_nums;
  bool auto_start = FLAGS_auto_start;

  db_ptr_set_t db_ptr_set = get_db_ptr_set(chain);
  tdb_ptr_t tdb_ptr = db_ptr_set.tdb_ptr;
  adb_ptr_t adb_ptr = db_ptr_set.adb_ptr;
  bdb_ptr_t bdb_ptr = db_ptr_set.bdb_ptr;

  if (auto_start) {
    start_ts = get_balance_db_start_ts(chain, balance_db_name);
    start_ts += seconds_interval;
    end_ts = neb::time_utils::get_universal_timestamp();
  }

  for (time_t ts = start_ts; ts < end_ts; ts += seconds_interval) {
    write_to_balance_db(tdb_ptr, adb_ptr, bdb_ptr, ts, thread_nums);
  }
  return 0;
}
