#include "blockchain.h"
#include "gtest_common.h"
#include "utils.h"
#include <gtest/gtest.h>

TEST(test_nebulas_transaction_db, test_append_transaction_to_db_full) {
  transaction_db_t nt_db(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                         std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  transaction_ptr_t transaction_db_ptr =
      std::make_shared<transaction_db_t>(nt_db);

  neb::block_height_t start_block = 1;
  neb::block_height_t end_block = 400000;

  for (neb::block_height_t h = start_block; h <= end_block; h++) {
    auto it_res =
        transaction_db_ptr
            ->read_success_and_failed_transaction_from_db_with_block_duration(
                h, h + 1);
    auto res = *it_res;

    auto it_v = ::neb::nebulas::nebulas_api::get_block_transactions_by_height(
        h, std::string());
    auto v = *it_v;
    std::vector<neb::transaction_info_t> rs;

    for (size_t i = 0; i < v.size(); ++i) {
      rs.push_back(v[i]);
      int32_t tx_status = v[i].template get<::neb::status>();

      auto it_events = ::neb::nebulas::nebulas_api::get_transaction_events(
          v[i], std::string(), tx_status);
      auto events = *it_events;
      for (auto it = events.begin(); it != events.end(); it++) {
        rs.push_back(*it);
      }
    }

    LOG(INFO) << h << ',' << res.size() << ',' << rs.size();
    EXPECT_TRUE(rs.size() == res.size());
    if (rs.size() != res.size()) {
      break;
    }
  }
}

