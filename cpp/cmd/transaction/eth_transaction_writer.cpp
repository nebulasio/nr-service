#include "blockchain.h"
#include "utils.h"
#include <chrono>
#include <thread>

DEFINE_int64(start_block, 0, "starting block height");
DEFINE_int64(end_block, 0, "ending block height");
DEFINE_int64(block_interval, 0, "block interval");
DEFINE_int32(thread_nums, 1, "the number of thread");
DEFINE_bool(auto_run, false, "run with append transactions");

typedef neb::eth::eth_transaction_db eth_transaction_db_t;
typedef std::shared_ptr<eth_transaction_db_t> eth_transaction_db_ptr_t;
typedef neb::account_db<neb::eth_db> eth_account_db_t;
typedef std::shared_ptr<eth_account_db_t> eth_account_db_ptr_t;

void para_run(const eth_transaction_db_ptr_t tx_ptr,
              neb::block_height_t start_block, neb::block_height_t end_block,
              int32_t thread_nums) {

  LOG(INFO) << start_block << ',' << end_block;

  auto start_time = std::chrono::high_resolution_clock::now();
  std::vector<std::thread> tv;

  int32_t block_interval = (end_block - start_block) / thread_nums;
  for (int32_t i = 0; i < thread_nums; i++) {
    int32_t s = start_block + i * block_interval;
    int32_t e = s + block_interval;

    std::thread t([&, s, e]() { tx_ptr->insert_transactions_to_db(s, e); });
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

int main(int argc, char *argv[]) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  neb::block_height_t start_block = FLAGS_start_block;
  neb::block_height_t end_block = FLAGS_end_block;
  neb::block_height_t block_interval = FLAGS_block_interval;
  int32_t thread_nums = FLAGS_thread_nums;
  bool auto_run = FLAGS_auto_run;

  eth_transaction_db_ptr_t tx_ptr = std::make_shared<eth_transaction_db_t>(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("ETH_DB"));

  if (auto_run) {
    tx_ptr->append_transactions();
    return 0;
  }

  while (start_block < end_block) {
    int32_t s = start_block;
    int32_t e = start_block + block_interval;

    para_run(tx_ptr, s, e, thread_nums);
    start_block += block_interval;
  }

  return 0;
}
