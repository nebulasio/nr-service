#include "blockchain.h"
#include "nr.h"
#include "utils.h"

DEFINE_string(chain, "nebulas", "chain name, nebulas or eth");
DEFINE_int32(start_ts, 0, "the first day end timestamp");
DEFINE_int32(end_ts, 1, "the last day end timestamp");
DEFINE_int32(thread_nums, 1, "the number of thread");

typedef neb::transaction_db_interface transaction_db_t;
typedef std::shared_ptr<transaction_db_t> tdb_ptr_t;
typedef neb::account_db_interface account_db_t;
typedef std::shared_ptr<account_db_t> adb_ptr_t;
typedef neb::nr_db_interface nr_db_t;
typedef std::shared_ptr<nr_db_t> ndb_ptr_t;
typedef neb::balance_db_interface balance_db_t;
typedef std::shared_ptr<balance_db_t> bdb_ptr_t;

typedef neb::transaction_db<neb::nebulas_db> nebulas_transaction_db_t;
typedef neb::account_db<neb::nebulas_db> nebulas_account_db_t;
typedef neb::nr_db<neb::nebulas_db> nebulas_nr_db_t;
typedef neb::balance_db<neb::nebulas_db> nebulas_balance_db_t;

typedef neb::transaction_db<neb::eth_db> eth_transaction_db_t;
typedef neb::account_db<neb::eth_db> eth_account_db_t;
typedef neb::nr_db<neb::eth_db> eth_nr_db_t;
typedef neb::balance_db<neb::eth_db> eth_balance_db_t;

struct db_ptr_set_t {
  std::string chain;
  tdb_ptr_t tdb_ptr;
  adb_ptr_t adb_ptr;
  ndb_ptr_t ndb_ptr;
  bdb_ptr_t bdb_ptr;
};

bool exists(const std::string &p) {
  return boost::filesystem::exists(boost::filesystem::path(p));
}

void write_date_nr(const db_ptr_set_t db_ptr_set, const std::string &date,
                   neb::block_height_t start_block,
                   neb::block_height_t end_block) {

  std::string chain = db_ptr_set.chain;
  tdb_ptr_t tdb_ptr = db_ptr_set.tdb_ptr;
  adb_ptr_t adb_ptr = db_ptr_set.adb_ptr;
  ndb_ptr_t ndb_ptr = db_ptr_set.ndb_ptr;
  bdb_ptr_t bdb_ptr = db_ptr_set.bdb_ptr;

  LOG(INFO) << "start block: " << start_block << " , end block: " << end_block;
  neb::rank_params_t rp{2000.0, 200000.0, 100.0, 1000.0, 0.75, 3.14};
  neb::nebulas_rank nr(tdb_ptr, adb_ptr, rp, start_block, end_block);

  std::string filename = "./cache/" + chain + '-' + date + ".dot";
  if (!exists(filename)) {
    // account inter transactions
    auto it_account_inter_txs =
        tdb_ptr->read_inter_transaction_from_db_with_duration(start_block,
                                                              end_block);
    auto account_inter_txs = *it_account_inter_txs;
    LOG(INFO) << "account to account: " << account_inter_txs.size();

    // graph
    auto it_txs_v =
        nr.split_transactions_by_x_block_interval(account_inter_txs);
    auto txs_v = *it_txs_v;

    neb::transaction_graph_ptr_t tg =
        std::make_shared<neb::transaction_graph>();

    // cache remove cycles result
    nr.filter_empty_transactions_this_interval(txs_v);
    std::vector<neb::transaction_graph_ptr_t> tgs =
        nr.build_transaction_graphs(txs_v);
    if (tgs.empty()) {
      return;
    }
    LOG(INFO) << "we have " << tgs.size() << " subgraphs.";
    for (auto it = tgs.begin(); it != tgs.end(); it++) {
      neb::transaction_graph_ptr_t ptr = *it;
      neb::graph_algo::remove_cycles_based_on_time_sequence(
          ptr->internal_graph());
      neb::graph_algo::merge_edges_with_same_from_and_same_to(
          ptr->internal_graph());
    }
    LOG(INFO) << "done with remove cycle.";

    tg = neb::graph_algo::merge_graphs(tgs);
    neb::graph_algo::merge_topk_edges_with_same_from_and_same_to(
        tg->internal_graph());
    LOG(INFO) << "done with merge graphs.";
    tg->write_to_graphviz(filename);
  }
}

void write_to_nebulas_rank_db(const db_ptr_set_t db_ptr_set, time_t end_ts) {

  tdb_ptr_t tdb_ptr = db_ptr_set.tdb_ptr;
  adb_ptr_t adb_ptr = db_ptr_set.adb_ptr;
  ndb_ptr_t ndb_ptr = db_ptr_set.ndb_ptr;
  bdb_ptr_t bdb_ptr = db_ptr_set.bdb_ptr;

  time_t seconds_of_day = 24 * 60 * 60;
  time_t seconds_of_ten_minute = 10 * 60;

  auto it_txs_in_end_last_minute =
      tdb_ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
          std::to_string(end_ts - seconds_of_ten_minute),
          std::to_string(end_ts));
  auto txs_in_end_last_minute = *it_txs_in_end_last_minute;
  if (txs_in_end_last_minute.empty()) {
    LOG(INFO) << "no transactions in end timestamp of last minute";
    return;
  }
  neb::block_height_t end_block =
      txs_in_end_last_minute.back().template get<::neb::height>();

  time_t start_ts = end_ts - seconds_of_day;
  auto it_txs_in_start_last_minute =
      tdb_ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
          std::to_string(start_ts - seconds_of_ten_minute),
          std::to_string(start_ts));
  auto txs_in_start_last_minute = *it_txs_in_start_last_minute;
  if (txs_in_start_last_minute.empty()) {
    LOG(INFO) << "no transactions in start timestamp of last minute";
    return;
  }
  neb::block_height_t start_block =
      txs_in_start_last_minute.back().template get<::neb::height>();

  std::string date = neb::time_utils::time_t_to_date(start_ts);
  LOG(INFO) << date;
  write_date_nr(db_ptr_set, date, start_block, end_block);
  return;
}

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
    nebulas_nr_db_t ndb(db_url, db_usrname, db_passwd, db_name);
    nebulas_balance_db_t bdb(db_url, db_usrname, db_passwd, db_name);

    tdb_ptr_t tdb_ptr = std::make_shared<nebulas_transaction_db_t>(tdb);
    adb_ptr_t adb_ptr = std::make_shared<nebulas_account_db_t>(adb);
    ndb_ptr_t ndb_ptr = std::make_shared<nebulas_nr_db_t>(ndb);
    bdb_ptr_t bdb_ptr = std::make_shared<nebulas_balance_db_t>(bdb);

    return db_ptr_set_t{chain, tdb_ptr, adb_ptr, ndb_ptr, bdb_ptr};
  }

  std::string db_name = std::getenv("ETH_DB");

  eth_transaction_db_t tdb(db_url, db_usrname, db_passwd, db_name);
  eth_account_db_t adb(db_url, db_usrname, db_passwd, db_name);
  eth_nr_db_t ndb(db_url, db_usrname, db_passwd, db_name);
  eth_balance_db_t bdb(db_url, db_usrname, db_passwd, db_name);

  tdb_ptr_t tdb_ptr = std::make_shared<eth_transaction_db_t>(tdb);
  adb_ptr_t adb_ptr = std::make_shared<eth_account_db_t>(adb);
  ndb_ptr_t ndb_ptr = std::make_shared<eth_nr_db_t>(ndb);
  bdb_ptr_t bdb_ptr = std::make_shared<eth_balance_db_t>(bdb);

  return db_ptr_set_t{chain, tdb_ptr, adb_ptr, ndb_ptr, bdb_ptr};
}

void para_run(const db_ptr_set_t db_ptr_set, time_t end_ts,
              int32_t thread_nums) {

  auto start_time = std::chrono::high_resolution_clock::now();
  std::vector<std::thread> tv;
  time_t seconds_of_day = 24 * 60 * 60;

  for (int32_t i = 0; i < thread_nums; i++) {
    time_t ts = end_ts + i * seconds_of_day;
    std::thread t(
        [&db_ptr_set, ts]() { write_to_nebulas_rank_db(db_ptr_set, ts); });
    tv.push_back(std::move(t));
  }

  for (auto &t : tv) {
    t.join();
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  LOG(INFO) << "time spend: "
            << std::chrono::duration_cast<std::chrono::seconds>(end_time -
                                                                start_time)
                   .count();
}

time_t get_start_ts_for_cache_dot_dir(const std::string &chain,
                                      int32_t start_ts) {
  time_t ts = start_ts;
  std::string date = neb::time_utils::time_t_to_date(ts);
  std::string filename =
      boost::str(boost::format("./cache/%1%-%2%.dot") % chain % date);

  while (exists(filename)) {
    ts += 24 * 3600;
    date = neb::time_utils::time_t_to_date(ts);
    LOG(INFO) << date;
    filename = boost::str(boost::format("./cache/%1%-%2%.dot") % chain % date);
  }

  return ts + 24 * 3600;
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::string chain = FLAGS_chain;
  int32_t start_ts = FLAGS_start_ts;
  int32_t end_ts = FLAGS_end_ts;
  int32_t thread_nums = FLAGS_thread_nums;

  db_ptr_set_t db_ptr_set = get_db_ptr_set(chain);
  time_t seconds_of_day = 24 * 60 * 60;

  start_ts = get_start_ts_for_cache_dot_dir(chain, start_ts);

  for (time_t ts = start_ts; ts < end_ts;
       ts += (seconds_of_day * thread_nums)) {
    para_run(db_ptr_set, ts, thread_nums);
  }
  return 0;
}
