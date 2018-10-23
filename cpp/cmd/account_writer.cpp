#include "blockchain.h"
#include "utils.h"

int main(int argc, char *argv[]) {

  typedef neb::nebulas::nebulas_account_db nebulas_account_db_t;
  typedef std::shared_ptr<nebulas_account_db_t> nebulas_account_db_ptr_t;

  nebulas_account_db_t db(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                          std::getenv("DB_PASSWORD"),
                          std::getenv("NEBULAS_DB"));
  nebulas_account_db_ptr_t ptr = std::make_shared<nebulas_account_db_t>(db);

  while (true) {
    ptr->append_account_to_db();
    LOG(INFO) << "waiting...";
    boost::asio::io_service io;
    boost::asio::deadline_timer t(io, boost::posix_time::seconds(60));
    t.wait();
  }
  return 0;
}
