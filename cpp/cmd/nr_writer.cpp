#include "blockchain.h"
#include "nr.h"
#include "utils.h"

DEFINE_string(chain, "nebulas", "chain name, nebulas or eth");
DEFINE_int64(start_block, 0, "the start block height");
DEFINE_int64(end_block, 1, "the end block height");
DEFINE_int32(start_ts, 0, "the first day end timestamp");
DEFINE_int32(end_ts, 1, "the last day end timestamp");

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

void nebulas_rank_detail(const tdb_ptr_t tdb_ptr, const adb_ptr_t adb_ptr,
                         const ndb_ptr_t ndb_ptr, const bdb_ptr_t bdb_ptr,
                         const std::string &date,
                         neb::block_height_t start_block,
                         neb::block_height_t end_block) {
  LOG(INFO) << "start block: " << start_block << " , end block: " << end_block;
  neb::rank_params_t rp{2000.0, 200000.0, 100.0, 1000.0, 0.75, 3.14};
  neb::nebulas_rank nr(tdb_ptr, adb_ptr, rp, start_block, end_block);

  // account inter transactions
  std::vector<neb::transaction_info_t> account_inter_txs =
      tdb_ptr->read_inter_transaction_from_db_with_duration(start_block,
                                                            end_block);
  LOG(INFO) << "account to account: " << account_inter_txs.size();

  // graph
  std::vector<std::vector<neb::transaction_info_t>> txs_v =
      nr.split_transactions_by_x_block_interval(account_inter_txs);
  nr.filter_empty_transactions_this_interval(txs_v);
  std::vector<neb::transaction_graph_ptr> tgs =
      nr.build_transaction_graphs(txs_v);
  if (tgs.empty()) {
    return;
  }
  LOG(INFO) << "we have " << tgs.size() << " subgraphs.";
  for (auto it = tgs.begin(); it != tgs.end(); it++) {
    neb::transaction_graph_ptr ptr = *it;
    neb::remove_cycles_based_on_time_sequence(ptr->internal_graph());
    neb::merge_edges_with_same_from_and_same_to(ptr->internal_graph());
  }
  LOG(INFO) << "done with remove cycle.";

  neb::transaction_graph_ptr tg = neb::merge_graphs(tgs);
  neb::merge_topk_edges_with_same_from_and_same_to(tg->internal_graph());
  LOG(INFO) << "done with merge graphs.";

  // median
  std::unordered_set<std::string> accounts =
      nr.get_normal_accounts(account_inter_txs);
  LOG(INFO) << "account size: " << accounts.size();

  std::unordered_map<neb::account_address_t, neb::account_balance_t>
      addr_balance;
  auto date_balance = bdb_ptr->read_balance_by_date(date);
  LOG(INFO) << "for date " << date << ", size: " << date_balance.size();
  for (auto &it : date_balance) {
    std::string address = it.template get<::neb::address>();
    std::string balance = it.template get<::neb::balance>();
    addr_balance.insert(std::make_pair(
        address, boost::lexical_cast<neb::account_balance_t>(balance)));
  }
  std::unordered_map<std::string, double> account_median =
      nr.get_account_balance_median(accounts, txs_v, adb_ptr, addr_balance);

  // degree and in_out amount
  std::unordered_map<std::string, neb::in_out_degree> in_out_degrees =
      neb::get_in_out_degrees(tg->internal_graph());
  std::unordered_map<std::string, int> degrees =
      neb::get_degree_sum(tg->internal_graph());
  std::unordered_map<std::string, neb::in_out_val> in_out_vals =
      neb::get_in_out_vals(tg->internal_graph());
  std::unordered_map<std::string, double> stakes =
      neb::get_stakes(tg->internal_graph());

  // weight and rank
  std::unordered_map<std::string, double> account_weight =
      nr.get_account_weight(in_out_vals, adb_ptr);
  std::unordered_map<std::string, double> account_rank =
      nr.get_account_rank(account_median, account_weight, rp);
  LOG(INFO) << "account rank size: " << account_rank.size();

  // std::cout
  //<< "addr,median,rank,in_degree,out_degree,degrees,in_val,out_val,vals"
  //<< std::endl;
  std::vector<neb::nr_info_t> infos;
  for (auto it = accounts.begin(); it != accounts.end(); it++) {
    std::string addr = *it;
    if (account_median.find(addr) == account_median.end() ||
        account_rank.find(addr) == account_rank.end() ||
        in_out_degrees.find(addr) == in_out_degrees.end() ||
        in_out_vals.find(addr) == in_out_vals.end() ||
        stakes.find(addr) == stakes.end()) {
      continue;
    }

    neb::nebulas::nas nas_in_val = neb::nebulas::nas_cast<neb::nebulas::nas>(
        neb::nebulas::wei(in_out_vals.find(addr)->second.in_val));
    neb::nebulas::nas nas_out_val = neb::nebulas::nas_cast<neb::nebulas::nas>(
        neb::nebulas::wei(in_out_vals.find(addr)->second.out_val));
    neb::nebulas::nas nas_stake = neb::nebulas::nas_cast<neb::nebulas::nas>(
        neb::nebulas::wei(stakes.find(addr)->second));

    // LOG(INFO) << addr << ',' << account_median[addr] << ','
    //<< account_rank[addr] << ',' << in_out_degrees[addr].in_degree
    //<< ',' << in_out_degrees[addr].out_degree << ',' << degrees[addr]
    //<< ',' << nas_in_val.value() << ',' << nas_out_val.value() << ','
    //<< nas_stake.value();

    neb::nr_info_t info;
    info.template set<::neb::date, ::neb::address, ::neb::median, ::neb::weight,
                      ::neb::score, ::neb::in_degree, ::neb::out_degree,
                      ::neb::degrees, ::neb::in_val, ::neb::out_val,
                      ::neb::in_outs>(
        date, addr, account_median[addr], account_weight[addr],
        account_rank[addr], in_out_degrees[addr].in_degree,
        in_out_degrees[addr].out_degree, degrees[addr], nas_in_val.value(),
        nas_out_val.value(), nas_stake.value());
    infos.push_back(info);
  }
  LOG(INFO) << "insert to db begin...";
  ndb_ptr->insert_date_nrs(infos);
  LOG(INFO) << "insert to db done";
}

void write_to_nebulas_rank_db(const tdb_ptr_t tdb_ptr, const adb_ptr_t adb_ptr,
                              const ndb_ptr_t ndb_ptr, const bdb_ptr_t bdb_ptr,
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
  nebulas_rank_detail(tdb_ptr, adb_ptr, ndb_ptr, bdb_ptr, date, start_block,
                      end_block);
  return;
}

struct db_ptr_set_t {
  tdb_ptr_t tdb_ptr;
  adb_ptr_t adb_ptr;
  ndb_ptr_t ndb_ptr;
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
    nebulas_nr_db_t ndb(db_url, db_usrname, db_passwd, db_name);
    nebulas_balance_db_t bdb(db_url, db_usrname, db_passwd, db_name);

    tdb_ptr_t tdb_ptr = std::make_shared<nebulas_transaction_db_t>(tdb);
    adb_ptr_t adb_ptr = std::make_shared<nebulas_account_db_t>(adb);
    ndb_ptr_t ndb_ptr = std::make_shared<nebulas_nr_db_t>(ndb);
    bdb_ptr_t bdb_ptr = std::make_shared<nebulas_balance_db_t>(bdb);

    return db_ptr_set_t{tdb_ptr, adb_ptr, ndb_ptr, bdb_ptr};
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

  return db_ptr_set_t{tdb_ptr, adb_ptr, ndb_ptr, bdb_ptr};
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::string chain = FLAGS_chain;
  int64_t start_block = FLAGS_start_block;
  int64_t end_block = FLAGS_end_block;
  int32_t start_ts = FLAGS_start_ts;
  int32_t end_ts = FLAGS_end_ts;

  db_ptr_set_t db_ptr_set = get_db_ptr_set(chain);
  tdb_ptr_t tdb_ptr = db_ptr_set.tdb_ptr;
  adb_ptr_t adb_ptr = db_ptr_set.adb_ptr;
  ndb_ptr_t ndb_ptr = db_ptr_set.ndb_ptr;
  bdb_ptr_t bdb_ptr = db_ptr_set.bdb_ptr;

  time_t seconds_of_day = 24 * 60 * 60;

  for (time_t ts = start_ts; ts < end_ts; ts += seconds_of_day) {
    write_to_nebulas_rank_db(tdb_ptr, adb_ptr, ndb_ptr, bdb_ptr, ts);
  }
  return 0;
}
