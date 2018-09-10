#include "blockchain.h"
#include "utils.h"

DEFINE_int64(start_block, 0, "the start block height");
DEFINE_int64(end_block, 1, "the end block height");
DEFINE_string(start_ts, "0", "the start timestamp");
DEFINE_string(end_ts, "1", "the end timestamp");

typedef neb::nebulas::nebulas_transaction_db nebulas_transaction_db_t;
typedef std::shared_ptr<nebulas_transaction_db_t> nebulas_transaction_db_ptr_t;
typedef neb::account_db<neb::nebulas_db> nebulas_account_db_t;
typedef std::shared_ptr<nebulas_account_db_t> nebulas_account_db_ptr_t;

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

void transaction_reader(const nebulas_transaction_db_ptr_t ptr,
                        neb::block_height_t start_block,
                        neb::block_height_t end_block) {
  // std::vector<neb::transaction_info_t> txs =
  // ptr->read_transaction_simplified_from_db_with_duration(320010, 320020);
  std::vector<neb::transaction_info_t> txs =
      ptr->read_success_and_failed_transaction_from_db_with_block_duration(
          start_block, end_block);
  for (auto it = txs.begin(); it != txs.end(); it++) {
    int32_t status = it->template get<::neb::status>();
    std::string from = it->template get<::neb::from>();
    std::string to = it->template get<::neb::to>();
    std::string tx_value = it->template get<::neb::tx_value>();
    int64_t height = it->template get<::neb::height>();
    std::string timestamp = it->template get<::neb::timestamp>();
    std::string type_from = it->template get<::neb::type_from>();
    std::string type_to = it->template get<::neb::type_to>();
    std::string gas_used = it->template get<::neb::gas_used>();
    std::string gas_price = it->template get<::neb::gas_price>();
    std::string contract_address = it->template get<::neb::contract_address>();
    LOG(INFO) << status << ',' << from << ',' << to << ',' << tx_value << ','
              << height << ',' << timestamp << ',' << type_from << ','
              << type_to << ',' << gas_used << ',' << gas_price << ','
              << contract_address;
  }
  LOG(INFO) << "transaction size: " << txs.size();
}

void account_reader(const nebulas_account_db_ptr_t ac_ptr,
                    neb::block_height_t start_block,
                    neb::block_height_t end_block) {
  ac_ptr->set_height_address_val(start_block, end_block);
  // double value = ac_ptr->get_normalized_value(100);
  // LOG(INFO) << value;
}

void ts_transaction_reader(const nebulas_transaction_db_ptr_t ptr,
                           const std::string &start_ts,
                           const std::string &end_ts) {
  // std::vector<neb::transaction_info_t> txs =
  // ptr->read_transaction_simplified_from_db_with_duration(320010, 320020);
  std::vector<neb::transaction_info_t> txs =
      ptr->read_success_and_failed_transaction_from_db_with_ts_duration(
          start_ts, end_ts);
  for (auto it = txs.begin(); it != txs.end(); it++) {
    int32_t status = it->template get<::neb::status>();
    std::string from = it->template get<::neb::from>();
    std::string to = it->template get<::neb::to>();
    std::string tx_value = it->template get<::neb::tx_value>();
    int64_t height = it->template get<::neb::height>();
    std::string timestamp = it->template get<::neb::timestamp>();
    std::string type_from = it->template get<::neb::type_from>();
    std::string type_to = it->template get<::neb::type_to>();
    std::string gas_used = it->template get<::neb::gas_used>();
    std::string gas_price = it->template get<::neb::gas_price>();
    std::string contract_address = it->template get<::neb::contract_address>();
    LOG(INFO) << status << ',' << from << ',' << to << ',' << tx_value << ','
              << height << ',' << timestamp << ',' << type_from << ','
              << type_to << ',' << gas_used << ',' << gas_price << ','
              << contract_address;
  }
  LOG(INFO) << "transaction size: " << txs.size();
}

void transaction_remove(const nebulas_transaction_db_ptr_t ptr,
                        neb::block_height_t start_block,
                        neb::block_height_t end_block) {
  ptr->remove_success_and_failed_transaction_from_db_with_block_duration(
      start_block, end_block);
}

int main(int argc, char *argv[]) {
  nebulas_transaction_db_t tdb(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  nebulas_transaction_db_ptr_t tx_ptr =
      std::make_shared<nebulas_transaction_db_t>(tdb);

  std::shared_ptr<::arangodb::fuerte::Connection> conn_ptr =
      tx_ptr->db_connection_ptr();

  nebulas_account_db_t adb(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                           std::getenv("DB_PASSWORD"),
                           std::getenv("NEBULAS_DB"));
  nebulas_account_db_ptr_t ac_ptr = std::make_shared<nebulas_account_db_t>(adb);

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  neb::block_height_t start_block = FLAGS_start_block;
  neb::block_height_t end_block = FLAGS_end_block;
  std::string start_ts = FLAGS_start_ts;
  std::string end_ts = FLAGS_end_ts;

  transaction_remove(tx_ptr, start_block, end_block);

  // ts_transaction_reader(tx_ptr, start_ts, end_ts);

  // account_reader(ac_ptr, start_block, end_block);
  return 0;
}
