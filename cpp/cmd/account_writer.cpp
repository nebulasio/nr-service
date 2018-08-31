#include "blockchain.h"
#include "utils.h"

int main(int argc, char *argv[]) {
  typedef neb::nebulas::nebulas_account_db nebulas_account_db_t;
  typedef std::shared_ptr<nebulas_account_db_t> nebulas_account_db_ptr_t;

  nebulas_account_db_t db(STR(DB_URL), STR(DB_USER_NAME), STR(DB_PASSWORD),
                          STR(NEBULAS_DB));
  nebulas_account_db_ptr_t ptr = std::make_shared<nebulas_account_db_t>(db);

  ptr->append_account_to_db();
  return 0;
}
