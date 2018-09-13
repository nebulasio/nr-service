#include "blockchain.h"
#include "gtest_common.h"
#include "utils.h"

#include <gtest/gtest.h>
#include <random>

TEST(test_eth_transaction_db, test_insert_transaction_to_db) {
  eth_transaction_db_t et_db(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                             std::getenv("DB_PASSWORD"), std::getenv("ETH_DB"));
  transaction_ptr_t transaction_db_ptr =
      std::make_shared<eth_transaction_db_t>(et_db);

  neb::block_height_t start_block = 5004000;
  neb::block_height_t end_block = 5005000;

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dis(start_block, end_block - 1);

  int32_t recheck_block_nums = 50;
  while (recheck_block_nums--) {
    neb::block_height_t height = dis(mt);
    LOG(INFO) << height;

    std::vector<neb::transaction_info_t> txs_block =
        ::neb::eth::get_block_transactions_by_height(height);
    std::string timestamp = txs_block.back().template get<::neb::timestamp>();

    std::vector<neb::transaction_info_t> txs_raw =
        ::neb::eth::trace_block(height);
    std::vector<neb::transaction_info_t> txs_db =
        transaction_db_ptr
            ->read_success_and_failed_transaction_from_db_with_block_duration(
                height, height);
    EXPECT_TRUE(txs_raw.size() == txs_db.size());

    auto it_raw = txs_raw.begin();
    auto it_db = txs_db.begin();

    for (; it_raw != txs_raw.end() && it_db != txs_db.end();
         it_raw++, it_db++) {
      EXPECT_TRUE(it_db->template get<::neb::timestamp>().compare(timestamp) ==
                  0);

      EXPECT_TRUE(it_raw->template get<::neb::height>() ==
                  it_db->template get<::neb::height>());
      EXPECT_TRUE(it_raw->template get<::neb::status>() ==
                  it_db->template get<::neb::status>());
      EXPECT_TRUE(it_raw->template get<::neb::from>().compare(
                      it_db->template get<::neb::from>()) == 0);
      EXPECT_TRUE(it_raw->template get<::neb::to>().compare(
                      it_db->template get<::neb::to>()) == 0);
      EXPECT_TRUE(it_raw->template get<::neb::tx_value>().compare(
                      it_db->template get<::neb::tx_value>()) == 0);
      EXPECT_TRUE(it_raw->template get<::neb::gas_used>().compare(
                      it_db->template get<::neb::gas_used>()) == 0);
    }
  }
}

