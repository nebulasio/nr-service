#include "nr.h"
#include "blockchain.h"
#include "utils.h"

DEFINE_int64(start_block, 0, "the start block height");
DEFINE_int64(end_block, 1, "the end block height");

typedef neb::transaction_db<neb::nebulas_db> nebulas_transaction_db_t;
typedef std::shared_ptr<nebulas_transaction_db_t> neb_tdb_ptr_t;
typedef neb::account_db<neb::nebulas_db> nebulas_account_db_t;
typedef std::shared_ptr<nebulas_account_db_t> neb_adb_ptr_t;
typedef neb::nr_db<neb::nebulas_db> nebulas_nr_db_t;
typedef std::shared_ptr<nebulas_nr_db_t> neb_ndb_ptr_t;

void nebulas_service(neb::block_height_t start_block,
                     neb::block_height_t end_block) {
  nebulas_transaction_db_t tdb(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  neb_tdb_ptr_t tdb_ptr = std::make_shared<nebulas_transaction_db_t>(tdb);
  auto txs =
      tdb_ptr->read_success_and_failed_transaction_from_db_with_block_duration(
          start_block, end_block);

  nebulas_account_db_t adb(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                           std::getenv("DB_PASSWORD"),
                           std::getenv("NEBULAS_DB"));
  neb_adb_ptr_t adb_ptr = std::make_shared<nebulas_account_db_t>(adb);

  neb::rank_params_t rp{2000.0, 200000.0, 100.0, 1000.0, 0.75, 3.14};
  neb::nebulas_rank nr(tdb_ptr, adb_ptr, rp, start_block, end_block);

  std::unordered_map<std::string, double> account_rank =
      nr.get_account_score_service();
  LOG(INFO) << account_rank.size();

  LOG(INFO) << "address,score";
  for (auto it = account_rank.begin(); it != account_rank.end(); it++) {
    LOG(INFO) << it->first << "," << it->second;
  }
}

void nebulas_rank_detail(const neb_tdb_ptr_t tdb_ptr,
                         const neb_adb_ptr_t adb_ptr,
                         const neb_ndb_ptr_t ndb_ptr,
                         neb::block_height_t start_block,
                         neb::block_height_t end_block) {
  auto txs = tdb_ptr->read_transaction_simplified_from_db_with_duration(
      start_block, end_block);
  LOG(INFO) << "transaction size: " << txs.size();
  neb::rank_params_t rp{2000.0, 200000.0, 100.0, 1000.0, 0.75, 3.14};
  neb::nebulas_rank nr(tdb_ptr, adb_ptr, rp, start_block, end_block);

  // account inter transactions
  std::vector<neb::transaction_info_t> account_inter_txs;
  for (auto it = txs.begin(); it != txs.end(); it++) {
    std::string from = it->template get<::neb::from>();
    std::string to = it->template get<::neb::to>();
    if (from.compare("normal") == 0 && to.compare("normal") == 0) {
      account_inter_txs.push_back(*it);
    }
  }

  // graph
  std::vector<std::vector<neb::transaction_info_t>> txs_v =
      nr.split_transactions_by_x_block_interval(account_inter_txs);
  nr.filter_empty_transactions_this_interval(txs_v);
  std::vector<neb::transaction_graph_ptr> tgs =
      nr.build_transaction_graphs(txs_v);
  for (auto it = tgs.begin(); it != tgs.end(); it++) {
    neb::transaction_graph_ptr ptr = *it;
    neb::remove_cycles_based_on_time_sequence(ptr->internal_graph());
    neb::merge_edges_with_same_from_and_same_to(ptr->internal_graph());
  }
  LOG(INFO) << "done with remove cycle.";

  neb::transaction_graph_ptr tg = neb::merge_graphs(tgs);
  neb::merge_topk_edges_with_same_from_and_same_to(tg->internal_graph());
  LOG(INFO) << "done with merge graphs.";

  std::unordered_set<std::string> accounts = nr.get_normal_accounts(txs);
  std::unordered_map<std::string, double> account_median =
      nr.get_account_balance_median(accounts, txs_v, adb_ptr);

  std::unordered_map<std::string, neb::in_out_degree> in_out_degrees =
      neb::get_in_out_degrees(tg->internal_graph());
  std::unordered_map<std::string, int> degrees =
      neb::get_degree_sum(tg->internal_graph());
  std::unordered_map<std::string, neb::in_out_val> in_out_vals =
      neb::get_in_out_vals(tg->internal_graph());
  std::unordered_map<std::string, double> stakes =
      neb::get_stakes(tg->internal_graph());

  std::unordered_map<std::string, double> account_weight =
      nr.get_account_weight(in_out_vals, adb_ptr);
  std::unordered_map<std::string, double> account_rank =
      nr.get_account_rank(account_median, account_weight, rp);

  // std::cout
  //<< "addr,median,rank,in_degree,out_degree,degrees,in_val,out_val,vals"
  //<< std::endl;
  for (auto it = accounts.begin(); it != accounts.end(); it++) {
    std::string addr = *it;
    if (account_median.find(addr) == account_median.end() ||
        account_rank.find(addr) == account_rank.end() ||
        in_out_degrees.find(addr) == in_out_degrees.end() ||
        in_out_vals.find(addr) == in_out_vals.end() ||
        stakes.find(addr) == stakes.end()) {
      continue;
    }

    LOG(INFO) << addr << ',' << account_median[addr] << ','
              << account_rank[addr] << ',' << in_out_degrees[addr].in_degree
              << ',' << in_out_degrees[addr].out_degree << ',' << degrees[addr]
              << ',' << in_out_vals[addr].in_val << ','
              << in_out_vals[addr].out_val << ',' << stakes[addr];

    neb::nr_info_t info;
    info.template set<::neb::address, ::neb::median, ::neb::weight,
                      ::neb::score, ::neb::in_degree, ::neb::out_degree,
                      ::neb::degrees, ::neb::in_val, ::neb::out_val,
                      ::neb::in_outs>(
        addr, account_median[addr], account_weight[addr], account_rank[addr],
        in_out_degrees[addr].in_degree, in_out_degrees[addr].out_degree,
        degrees[addr], in_out_vals[addr].in_val, in_out_vals[addr].out_val,
        stakes[addr]);
    ndb_ptr->insert_document_to_collection(info);
  }
}

void write_to_nebulas_rank_db() {

  nebulas_transaction_db_t tdb(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  neb_tdb_ptr_t tdb_ptr = std::make_shared<nebulas_transaction_db_t>(tdb);

  nebulas_account_db_t adb(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                           std::getenv("DB_PASSWORD"),
                           std::getenv("NEBULAS_DB"));
  neb_adb_ptr_t adb_ptr = std::make_shared<nebulas_account_db_t>(adb);

  nebulas_nr_db_t ndb(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  neb_ndb_ptr_t ndb_ptr = std::make_shared<nebulas_nr_db_t>(ndb);

  time_t seconds_of_day = 24 * 60 * 60;
  time_t seconds_of_minute = 60;

  time_t end_ts = neb::get_universal_timestamp();
  std::vector<neb::transaction_info_t> txs_in_end_last_minute =
      tdb_ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
          std::to_string(end_ts - seconds_of_minute), std::to_string(end_ts));
  if (txs_in_end_last_minute.empty()) {
    LOG(INFO) << "no transactions in end timestamp of last minute";
    return;
  }
  neb::block_height_t end_block =
      txs_in_end_last_minute.back().template get<::neb::height>();

  time_t start_ts = end_ts - seconds_of_day;
  std::vector<neb::transaction_info_t> txs_in_start_last_minute =
      tdb_ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
          std::to_string(start_ts - seconds_of_minute),
          std::to_string(start_ts));
  if (txs_in_start_last_minute.empty()) {
    LOG(INFO) << "no transactions in start timestamp of last minute";
    return;
  }
  neb::block_height_t start_block =
      txs_in_start_last_minute.back().template get<::neb::height>();

  nebulas_rank_detail(tdb_ptr, adb_ptr, ndb_ptr, start_block, end_block);
  return;
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  int64_t start_block = FLAGS_start_block;
  int64_t end_block = FLAGS_end_block;

  // LOG(INFO) << start_block << ',' << end_block;
  // nebulas_service(start_block, end_block);

  time_t seconds_of_day = 24 * 60 * 60;
  while (true) {
    time_t time_now = neb::get_universal_timestamp();
    if (time_now % seconds_of_day < 60) {
      write_to_nebulas_rank_db();
      LOG(INFO) << "waiting...";
    }
    boost::asio::io_service io;
    boost::asio::deadline_timer t(io, boost::posix_time::seconds(30));
    t.wait();
  }
  return 0;
}
