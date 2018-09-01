#include "blockchain.h"
#include "utils.h"

DEFINE_string(address, "", "nebulas address");

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::string address = FLAGS_address;

  LOG(INFO) << "is contract address: "
            << neb::nebulas::is_contract_address(address);
  LOG(INFO) << "address length: " << address.length();
  LOG(INFO) << "address size: " << address.size();

  std::vector<unsigned char> v;
  bool ret = neb::decode_base58(address, v);
  LOG(INFO) << "ret: " << ret;

  for (size_t i = 0; i < v.size(); i++) {
    std::cout << "index " << i << ':';
    int32_t ch = v[i];
    std::cout << ch << std::endl;
  }
  return 0;
}
