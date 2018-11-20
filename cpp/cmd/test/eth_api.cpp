#include "blockchain.h"
#include "utils.h"

DEFINE_string(address, "", "eth address");
DEFINE_int64(block_height, 0, "block height");

int main(int argc, char *argv[]) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::string address = FLAGS_address;
  neb::block_height_t block_height = FLAGS_block_height;

  LOG(INFO) << neb::eth::eth_api::get_address_type(address, "0x0");
  LOG(INFO) << neb::eth::eth_api::get_block_height();
  neb::eth::eth_api::get_block_transactions_by_height(block_height);

  return 0;
}
