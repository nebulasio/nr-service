#include "blockchain.h"
#include "cache.h"
#include "utils.h"

DEFINE_int64(start_block, 0, "the start block height");
DEFINE_int64(end_block, 1, "the end block height");
DEFINE_int32(cache_size, 24 * 3600 / 15, "LRU cache max size");

void example() {
  neb::lru_cache<int32_t, int32_t> cache(3);
  int32_t a[] = {7, 0, 1, 2, 0, 3, 0, 4};

  auto f = [](const neb::key_value_pair_t<int32_t, int32_t> &p) {
    std::cout << p.m_key << ',';
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
  nebulas_transaction_db_t tdb(STR(DB_URL), STR(DB_USER_NAME), STR(DB_PASSWORD),
                               STR(NEBULAS_DB));
  nebulas_transaction_db_ptr_t tx_ptr =
      std::make_shared<nebulas_transaction_db_t>(tdb);

  std::vector<neb::transaction_info_t> txs =
      tx_ptr->read_success_and_failed_transaction_from_db_with_duration(
          height, height + 3600 / 15);
  LOG(INFO) << "read ahead transaction size: " << txs.size();
  std::unordered_map<neb::block_height_t, std::vector<neb::transaction_info_t>>
      height_and_txs;

  for (auto it = txs.begin(); it != txs.end(); it++) {
    neb::block_height_t height = it->template get<::neb::height>();
    height_and_txs[height].push_back(*it);
    // auto ite = height_and_txs.find(height);
    // if (ite != height_and_txs.end()) {
    // std::vector<neb::transaction_info_t> &v = ite->second;
    // v.push_back(*it);
    //} else {
    // std::vector<neb::transaction_info_t> v({*it});
    // height_and_txs.insert(std::make_pair(height, v));
    //}
  }

  for (auto it = height_and_txs.begin(); it != height_and_txs.end(); it++) {
    cache.set(it->first, it->second);
  }
}

void transaction_cache(int32_t cache_size) {

  neb::lru_cache<neb::block_height_t, std::vector<neb::transaction_info_t>>
      cache(cache_size);

  neb::block_height_t start_block;
  neb::block_height_t end_block;

  while (true) {
    std::cin >> start_block >> end_block;
    std::vector<neb::transaction_info_t> txs;

    for (neb::block_height_t h = start_block; h <= end_block; h++) {
      std::vector<neb::transaction_info_t> rs;
      if (!cache.get(h, rs)) {
        LOG(INFO) << "cache missed, reading from db";
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
