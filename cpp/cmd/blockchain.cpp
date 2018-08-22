#include "blockchain.h"
#include "utils.h"

typedef neb::nebulas::nebulas_transaction_db nebulas_transaction_db_t;
typedef std::shared_ptr<nebulas_transaction_db_t> nebulas_transaction_db_ptr_t;

void aql_query_result_traverse(const nebulas_transaction_db_ptr_t ptr) {
  auto result = ptr->aql_query("for item in transaction return item");
  auto documents = result->slices().front().get("result");
  for (size_t i = 0; i < documents.length(); i++) {
    auto doc = documents.at(i);
    LOG(INFO) << doc.length();
    for (size_t j = 0; j < doc.length(); j++) {
      LOG(INFO) << doc.keyAt(j).copyString() << ','
                << doc.valueAt(j).copyString();
    }
  }
}

void aql_query(const nebulas_transaction_db_ptr_t ptr) {
  auto result = ptr->aql_query(
      "for h in height sort h.block_height desc limit 1 return h.block_height");
  auto documents = result->slices().front().get("result");
  LOG(INFO) << documents;
  LOG(INFO) << documents.length();
  for (size_t i = 0; i < documents.length(); i++) {
    auto doc = documents.at(i);
    LOG(INFO) << doc.getInt();
  }
}

int main(int argc, char *argv[]) {
  nebulas_transaction_db_t db(STR(DB_URL), STR(DB_USER_NAME), STR(DB_PASSWORD),
                              STR(NEBULAS_DB));
  nebulas_transaction_db_ptr_t ptr =
      std::make_shared<nebulas_transaction_db_t>(db);
  std::shared_ptr<::arangodb::fuerte::Connection> conn_ptr =
      ptr->transaction_db_connection_ptr();

  aql_query(ptr);
  return 0;
}
