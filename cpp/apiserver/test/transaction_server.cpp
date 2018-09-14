#include "api/transaction_apiserver.h"

int main(int argc, char *argv[]) {

  transaction_apiserver<neb::nebulas_db> server("server");

  std::unordered_map<std::string, std::string> params = {
      {"start_block", "400000"}, {"end_block", "400600"}};
  server.on_api_transaction(params);
  return 0;
}
