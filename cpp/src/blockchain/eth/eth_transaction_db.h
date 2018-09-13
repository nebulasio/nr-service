#pragma once
#include "blockchain/transaction_db.h"

namespace neb {
namespace eth {
class eth_transaction_db : public transaction_db<eth_db> {

public:
  eth_transaction_db();
  eth_transaction_db(const std::string &url, const std::string &usrname,
                     const std::string &passwd, const std::string &dbname);

  void insert_transactions_to_db(block_height_t start_block,
                                 block_height_t end_block);

  void clean_transaction_db();

private:
  void set_transactions(const std::vector<transaction_info_t> &txs,
                        std::vector<transaction_info_t> &internal_txs);
  std::string get_address_type(const std::string &address);

  void insert_block_transactions(const std::vector<transaction_info_t> &txs);
  void insert_transaction(VPackBuilder &builder_arr,
                          const transaction_info_t &info);

private:
  typedef std::string address_t;
  typedef std::string type_t;

  std::unordered_map<address_t, type_t> m_addr_and_type;
};
} // namespace eth
} // namespace neb
