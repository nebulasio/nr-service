#include "nr.h"
#include "blockchain.h"
#include "utils.h"

DEFINE_int64(start_block, 0, "the start block height");
DEFINE_int64(end_block, 1, "the end block height");

typedef neb::transaction_db<neb::nebulas_db> nebulas_transaction_db_t;
typedef std::shared_ptr<nebulas_transaction_db_t> neb_tdb_ptr_t;
typedef neb::account_db<neb::nebulas_db> nebulas_account_db_t;
typedef std::shared_ptr<nebulas_account_db_t> neb_adb_ptr_t;

void nebulas_service(neb::block_height_t start_block,
                     neb::block_height_t end_block) {
  nebulas_transaction_db_t tdb(STR(DB_URL), STR(DB_USER_NAME), STR(DB_PASSWORD),
                               STR(NEBULAS_DB));
  neb_tdb_ptr_t tdb_ptr = std::make_shared<nebulas_transaction_db_t>(tdb);
  auto txs = tdb_ptr->read_success_and_failed_transaction_from_db_with_duration(
      start_block, end_block);

  nebulas_account_db_t adb(STR(DB_URL), STR(DB_USER_NAME), STR(DB_PASSWORD),
                           STR(NEBULAS_DB));
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

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  int64_t start_block = FLAGS_start_block;
  int64_t end_block = FLAGS_end_block;

  LOG(INFO) << start_block << ',' << end_block;
  nebulas_service(start_block, end_block);
  return 0;
}
