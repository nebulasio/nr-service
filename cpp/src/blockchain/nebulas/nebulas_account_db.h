#pragma once
#include "blockchain/account_db.h"

namespace neb {
namespace nebulas {

class nebulas_account_db : public account_db<nebulas_db> {

public:
  nebulas_account_db();
  nebulas_account_db(const std::string &url, const std::string &usrname,
                     const std::string &passwd, const std::string &dbname);

  void append_account_to_db();

protected:
  block_height_t get_max_height_from_account();
  block_height_t get_max_height_from_transaction();

  void set_coinbase_account();

  std::vector<std::pair<account_address_t, account_info_t>>
  sort_update_info_by_height(
      const std::unordered_map<account_address_t, account_info_t>
          &to_update_info);

  void insert_document_to_collection(const account_info_t &info);

}; // end class nebulas_account_db
} // end namespace nebulas
} // end namespace neb
