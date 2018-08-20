#include "blockchain/nebulas/nebulas_transaction_db.h"
#include "blockchain/transaction_db.h"

namespace neb {
namespace nebulas {

nebulas_transaction_db::nebulas_transaction_db() {}
nebulas_transaction_db::nebulas_transaction_db(const std::string &url,
                                               const std::string &usrname,
                                               const std::string &passwd)
    : transaction_db<nebulas_db>(url, usrname, passwd) {}

void nebulas_transaction_db::append_transaction_to_db() {
  // auto ret = sql_table_t::template select<height>(m_engine_ptr.get())
  //.template order_by<height, ::neb::sql::desc>()
  //.limit(1)
  //.eval();
  // block_height_t last_height = 0;
  // if (!ret.empty()) {
  // last_height = ret[0].template get<height>();
  //}

  // block_height_t current_height = ::neb::nebulas::get_block_height();

  // for (int h = last_height + 1; h < current_height; h++) {

  // LOG(INFO) << h;
  // std::string block_timestamp =
  //::neb::nebulas::get_block_timestamp_by_height(h);
  // if (block_timestamp.compare(std::string()) == 0) {
  // continue;
  //}
  // std::vector<transaction_info_t> v =
  //::neb::nebulas::get_block_transactions_by_height(h, block_timestamp);

  // typename sql_table_t::row_collection_type rs;
  // for (size_t i = 0; i < v.size(); ++i) {
  // rs.push_back(v[i]);
  // int32_t tx_status = v[i].template get<::neb::status>();

  // std::vector<transaction_info_t> events =
  //::neb::nebulas::get_transaction_events(v[i], block_timestamp,
  // tx_status);
  // for (auto it = events.begin(); it != events.end(); it++) {
  // rs.push_back(*it);
  //}
  //}
  // sql_table_t::insert_or_replace_rows(m_engine_ptr.get(), rs);
  //}
}

} // end namespace nebulas
} // end namespace neb
