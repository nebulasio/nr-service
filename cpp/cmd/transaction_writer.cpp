#include "blockchain.h"
#include "utils.h"

int main(int argc, char *argv[]) {

  typedef neb::nebulas::nebulas_transaction_db nebulas_transaction_db_t;
  typedef std::shared_ptr<nebulas_transaction_db_t>
      nebulas_transaction_db_ptr_t;

  nebulas_transaction_db_t db(STR(DB_URL), STR(DB_USER_NAME), STR(DB_PASSWORD),
                              STR(NEBULAS_DB));
  nebulas_transaction_db_ptr_t ptr =
      std::make_shared<nebulas_transaction_db_t>(db);

  std::shared_ptr<::arangodb::fuerte::Connection> conn_ptr =
      ptr->transaction_db_connection_ptr();

  ptr->append_transaction_to_graph();
  return 0;
}
