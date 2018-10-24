#pragma once
#include "blockchain/account_db.h"

namespace neb {
namespace eth {

class eth_account_db : public account_db<eth_db> {
public:
  eth_account_db();
  eth_account_db(const std::string &url, const std::string &usrname,
                 const std::string &passwd, const std::string &dbname);

  void insert_account_to_db(block_height_t start_block,
                            block_height_t end_block);

protected:
  void insert_account(const account_info_t &info);

  void set_accounts(
      const transaction_info_t &info,
      std::unordered_map<account_address_t, account_info_t> &addr_and_info);

  std::shared_ptr<std::vector<std::pair<account_address_t, account_info_t>>>
  sort_account_info_by_height(
      const std::unordered_map<account_address_t, account_info_t>
          &addr_and_account_info);
};
} // namespace eth
} // namespace neb
