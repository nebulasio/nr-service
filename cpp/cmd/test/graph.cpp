#include "graph.h"
#include "blockchain.h"
#include "nr.h"
#include "utils.h"

DEFINE_int64(start_block, 0, "the start block height");
DEFINE_int64(end_block, 1, "the end block height");

typedef neb::nebulas::nebulas_transaction_db nebulas_transaction_db_t;
typedef std::shared_ptr<nebulas_transaction_db_t> nebulas_transaction_db_ptr_t;

template <class TransInfo>
neb::transaction_graph_ptr_t
build_graph_from_transactions(const std::vector<TransInfo> &trans) {
  neb::transaction_graph_ptr_t ret = std::make_shared<neb::transaction_graph>();

  for (auto ite = trans.begin(); ite != trans.end(); ite++) {
    std::string from = ite->template get<::neb::from>();
    std::string to = ite->template get<::neb::to>();
    std::string tx_value = ite->template get<::neb::tx_value>();
    double value = std::stod(tx_value);
    double timestamp = std::stod(ite->template get<::neb::timestamp>());
    ret->add_edge(from, to, value, timestamp);
  }
  return ret;
}

int main(int argc, char *argv[]) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  neb::block_height_t start_block = FLAGS_start_block;
  neb::block_height_t end_block = FLAGS_end_block;

  nebulas_transaction_db_ptr_t tx_ptr =
      std::make_shared<nebulas_transaction_db_t>(
          std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
          std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));

  auto txs_ptr =
      tx_ptr->read_succ_and_failed_inter_transaction_from_db_with_duration(
          start_block, end_block);

  auto tgp_ptr_raw = build_graph_from_transactions(*txs_ptr);
  tgp_ptr_raw->write_to_graphviz("xx.dot");

  neb::transaction_graph_ptr_t tgp = std::make_shared<neb::transaction_graph>();
  tgp->read_from_graphviz("xx.dot");
  tgp->write_to_graphviz("yy.dot");

  return 0;
}
