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

  neb::block_height_t start_block = 1;
  neb::block_height_t end_block = 2000000;

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dis(start_block, end_block);

  int32_t recheck_block_nums = 1;
  while (true) {
    neb::block_height_t height = dis(mt);

    std::vector<neb::transaction_info_t> txs_block =
        ::neb::eth::get_block_transactions_by_height(height);
    std::string timestamp = txs_block.back().template get<::neb::timestamp>();

    std::vector<neb::transaction_info_t> txs_raw =
        ::neb::eth::trace_block(height);
    std::vector<neb::transaction_info_t> txs_db =
        transaction_db_ptr
            ->read_success_and_failed_transaction_from_db_with_block_duration(
                height, height + 1);
    sort(txs_db.begin(), txs_db.end(),
         [](const neb::transaction_info_t &info1,
            neb::transaction_info_t &info2) {
           int64_t tx_id1 = info1.template get<::neb::tx_id>();
           int64_t tx_id2 = info2.template get<::neb::tx_id>();
           return tx_id1 < tx_id2;
         });
    LOG(INFO) << recheck_block_nums++ << ',' << height << ',' << txs_db.size();
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

      std::string from = it_db->template get<::neb::from>();
      std::string to = it_db->template get<::neb::to>();
      EXPECT_TRUE(it_raw->template get<::neb::from>().compare(from) == 0);
      EXPECT_TRUE(it_raw->template get<::neb::to>().compare(to) == 0);
      EXPECT_TRUE(it_raw->template get<::neb::tx_value>().compare(
                      it_db->template get<::neb::tx_value>()) == 0);
      EXPECT_TRUE(it_raw->template get<::neb::gas_used>().compare(
                      it_db->template get<::neb::gas_used>()) == 0);

      std::string type_from = it_db->template get<::neb::type_from>();
      std::string type_to = it_db->template get<::neb::type_to>();

      auto cmp_type_from =
          type_from.compare(::neb::eth::get_address_type(from));
      auto cmp_type_to = type_to.compare(::neb::eth::get_address_type(to));
      EXPECT_TRUE(cmp_type_from == 0);
      EXPECT_TRUE(cmp_type_to == 0);

      std::ofstream file;
      file.open("check.csv");
      file << "height,from,to,type_from,type_to\n";
      if (cmp_type_from != 0 || cmp_type_to != 0) {
        file << it_db->template get<::neb::height>() << ','
             << it_db->template get<::neb::from>() << ','
             << it_db->template get<::neb::to>() << ','
             << it_db->template get<::neb::type_from>() << ','
             << it_db->template get<::neb::type_to>() << '\n';
      }

      // LOG(INFO) << from << ',' << to << ',' << type_from << ',' << type_to;
    }
  }
}

