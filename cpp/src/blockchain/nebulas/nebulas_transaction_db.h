#pragma once
#include "blockchain/transaction_db.h"

namespace neb {
namespace nebulas {

class nebulas_transaction_db : public transaction_db<nebulas_db> {

public:
  nebulas_transaction_db();
  nebulas_transaction_db(const std::string &url, const std::string &usrname,
                         const std::string &passwd, const std::string &dbname);

  void append_transaction_to_graph();

private:
  void append_transaction_graph_vertex_and_edge_by_block(
      const std::vector<transaction_info_t> &rs);
  template <class T>
  void insert_documents_to_collection(const std::string &collection_name,
                                      const std::vector<T> &documents,
                                      int32_t payload_size = 50);
  template <class T>
  void insert_document(VPackBuilder &builder_arr,
                       const std::string &collection_name, const T &document);

  block_height_t get_max_height_from_db();
  int64_t get_max_tx_id_from_db();

}; // end class neubulas_transaction_db

} // end namespace nebulas
} // end namespace neb
