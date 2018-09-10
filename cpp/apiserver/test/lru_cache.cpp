#include "cache/lru_cache.h"
#include "blockchain.h"
#include "utils.h"

DEFINE_int64(start_block, 0, "the start block height");
DEFINE_int64(end_block, 1, "the end block height");
DEFINE_int32(cache_size, 24 * 3600 / 15, "LRU cache max size");

void example() {
  neb::lru_cache<int32_t, int32_t> cache;
  int32_t a[] = {7, 0, 1, 2, 0, 3, 0, 4};

  auto f = [](int32_t key, int32_t value) {
    std::cout << key << " - " << value << ',';
  };

  for (size_t i = 0; i < sizeof(a) / sizeof(a[0]); i++) {
    int32_t value;
    if (!cache.get(a[i], value)) {
      cache.set(a[i], 0);
    }
    cache.watch(f);
    std::cout << std::endl;
  }
}

typedef neb::nebulas::nebulas_transaction_db nebulas_transaction_db_t;
typedef std::shared_ptr<nebulas_transaction_db_t> nebulas_transaction_db_ptr_t;

void set_cache(neb::lru_cache<neb::block_height_t,
                              std::vector<neb::transaction_info_t>> &cache,
               neb::block_height_t height) {
  nebulas_transaction_db_t tdb(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  nebulas_transaction_db_ptr_t tx_ptr =
      std::make_shared<nebulas_transaction_db_t>(tdb);

  std::vector<neb::transaction_info_t> txs =
      tx_ptr->read_success_and_failed_transaction_from_db_with_block_duration(
          height, height);
  LOG(INFO) << "read ahead transaction size: " << txs.size();
  std::unordered_map<neb::block_height_t, std::vector<neb::transaction_info_t>>
      height_and_txs;

  for (auto it = txs.begin(); it != txs.end(); it++) {
    neb::block_height_t height = it->template get<::neb::height>();
    height_and_txs[height].push_back(*it);
  }

  for (auto it = height_and_txs.begin(); it != height_and_txs.end(); it++) {
    cache.set(it->first, it->second);
  }
}

void transaction_cache(int32_t) {

  neb::lru_cache<neb::block_height_t, std::vector<neb::transaction_info_t>>
      cache;

  neb::block_height_t start_block;
  neb::block_height_t end_block;

  while (true) {
    std::cin >> start_block >> end_block;
    std::vector<neb::transaction_info_t> txs;

    for (neb::block_height_t h = start_block; h <= end_block; h++) {
      std::vector<neb::transaction_info_t> rs;
      if (!cache.get(h, rs)) {
        LOG(INFO) << "cache missed, reading from db, height:" << h;
        set_cache(cache, h);

        if (!cache.get(h, rs)) {
          LOG(INFO) << "db missed";
        }
      }
      txs.insert(txs.end(), rs.begin(), rs.end());
    }
    LOG(INFO) << "transaction size: " << txs.size();
  }
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  int32_t cache_size = FLAGS_cache_size;

  transaction_cache(cache_size);

  return 0;
}
