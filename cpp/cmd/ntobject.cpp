#include "blockchain.h"
#include "utils.h"

int main(int argc, char *argv[]) {

  neb::transaction_info_t info;
  info.set<neb::from, neb::to, neb::tx_value>("source", "target", "100000000");

  LOG(INFO) << info.get<neb::from>() << ',' << info.get<neb::to>();

  neb::transaction_table_t info_array;
  info_array.push_back(info);
  return 0;
}
