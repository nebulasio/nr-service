#include "blockchain.h"
#include "gtest_common.h"
#include "utils.h"
#include <gtest/gtest.h>

TEST(test_nebulas_account_db, test_append_account_to_db_full) {
  nebulas_account_db_t na_db(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                             std::getenv("DB_PASSWORD"),
                             std::getenv("NEBULAS_DB"));
  account_ptr_t account_db_ptr = std::make_shared<nebulas_account_db_t>(na_db);

  auto it_rs = account_db_ptr->read_account_from_db_sort_by_balance_desc();
  auto rs = *it_rs;

  auto it_begin = rs.begin();
  for (auto it = rs.begin(); it != rs.end(); it++) {
    std::string address = it->template get<::neb::address>();
    neb::block_height_t height = it->template get<::neb::height>();
    std::string balance = it->template get<::neb::balance>();
    std::string account_type = it->template get<::neb::account_type>();
    LOG(INFO) << it - it_begin << ',' << address << ',' << height << ','
              << balance << ',' << account_type;

    std::pair<std::string, int> account_state =
        neb::nebulas::get_account_state(address, height);

    EXPECT_TRUE(balance.compare(account_state.first) == 0);

    std::map<std::string, int32_t> type_and_code(
        {{"normal", 0x57}, {"contract", 0x58}});

    auto ret = type_and_code.find(account_type);
    EXPECT_TRUE(ret->second == account_state.second);

    if (ret->second != account_state.second) {
      break;
    }

    // if (it - rs.begin() > 10000) {
    // break;
    //}
  }
}

