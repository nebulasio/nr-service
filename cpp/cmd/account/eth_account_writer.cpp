#include "blockchain.h"
#include "utils.h"

DEFINE_int64(start_block, 0, "starting block height");
DEFINE_int64(end_block, 0, "ending block height");

typedef neb::eth::eth_account_db eth_account_db_t;
typedef std::shared_ptr<eth_account_db_t> eth_account_db_ptr_t;

int main(int argc, char *argv[]) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  neb::block_height_t start_block = FLAGS_start_block;
  neb::block_height_t end_block = FLAGS_end_block;

  eth_account_db_t adb(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                       std::getenv("DB_PASSWORD"), std::getenv("ETH_DB"));
  eth_account_db_ptr_t ac_ptr = std::make_shared<eth_account_db_t>(adb);

  ac_ptr->insert_account_to_db(start_block, end_block);
  return 0;
}
