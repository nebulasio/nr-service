#include "blockchain.h"
#include "gtest_common.h"
#include "utils.h"
#include <gtest/gtest.h>

TEST(test_nebulas_account_db, test_append_account_to_db_full) {
  nebulas_account_db_t na_db(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                             std::getenv("DB_PASSWORD"),
                             std::getenv("NEBULAS_DB"));
  account_ptr_t account_db_ptr = std::make_shared<nebulas_account_db_t>(na_db);

  std::vector<neb::account_info_t> rs = account_db_ptr->read_account_from_db();
  for (auto it = rs.begin(); it != rs.end(); it++) {
    std::string address = it->template get<::neb::address>();
    neb::block_height_t height = it->template get<::neb::height>();
    std::string balance = it->template get<::neb::balance>();
    std::string account_type = it->template get<::neb::account_type>();
    LOG(INFO) << address << ',' << height << ',' << balance << ','
              << account_type;

    std::pair<std::string, int> account_state =
        neb::nebulas::get_account_state(height, address);

    EXPECT_TRUE(balance.compare(account_state.first) == 0);
    if (account_type.compare("normal") == 0) {
      EXPECT_TRUE(account_state.second == 0x57);
    } else if (account_type.compare("contract") == 0) {
      EXPECT_TRUE(account_state.second == 0x58);
    }

    if (it - rs.begin() > 1000) {
      break;
    }
  }
}

