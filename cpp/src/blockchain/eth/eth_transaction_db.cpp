#include "blockchain/eth/eth_transaction_db.h"
#include "blockchain/eth/eth_api.h"

namespace neb {
namespace eth {

eth_transaction_db::eth_transaction_db() {}
eth_transaction_db::eth_transaction_db(const std::string &url,
                                       const std::string &usrname,
                                       const std::string &passwd,
                                       const std::string &dbname)
    : transaction_db<eth_db>(url, usrname, passwd, dbname) {}

void eth_transaction_db::insert_transaction(VPackBuilder &builder_arr,
                                            const transaction_info_t &info) {
  auto it = &info;

  VPackBuilder builder;
  builder.openObject();
  builder.add("status", VPackValue(it->template get<::neb::status>()));
  builder.add("from", VPackValue(it->template get<::neb::from>()));
  builder.add("timestamp", VPackValue(it->template get<::neb::timestamp>()));
  builder.add("gas_used", VPackValue(it->template get<::neb::gas_used>()));
  builder.add("tx_value", VPackValue(it->template get<::neb::tx_value>()));
  builder.add("data", VPackValue(it->template get<::neb::data>()));
  builder.add("to", VPackValue(it->template get<::neb::to>()));
  builder.add("contract_address",
              VPackValue(it->template get<::neb::contract_address>()));
  builder.add("hash", VPackValue(it->template get<::neb::hash>()));
  builder.add("gas_price", VPackValue(it->template get<::neb::gas_price>()));
  builder.add("tx_type", VPackValue(it->template get<::neb::tx_type>()));
  builder.add("gas_limit", VPackValue(it->template get<::neb::gas_limit>()));
  builder.add("height", VPackValue(it->template get<::neb::height>()));
  builder.add("type_from", VPackValue(it->template get<::neb::type_from>()));
  builder.add("type_to", VPackValue(it->template get<::neb::type_to>()));
  builder.close();
  builder_arr.add(builder.slice());
}

void eth_transaction_db::insert_block_transactions(
    const std::vector<transaction_info_t> &txs) {

  auto request = ::arangodb::fuerte::createRequest(
      ::arangodb::fuerte::RestVerb::Post,
      "/_db/" + m_dbname + "/_api/document/transaction");

  VPackBuilder builder_arr;
  builder_arr.openArray();

  for (auto tx : txs) {
    insert_transaction(builder_arr, tx);
  }

  builder_arr.close();
  request->addVPack(builder_arr.slice());
  m_connection_ptr->sendRequest(std::move(request));
}

void eth_transaction_db::insert_transactions_to_db(block_height_t start_block,
                                                   block_height_t end_block) {
  for (int h = start_block; h < end_block; h++) {

    // LOG(INFO) << h;

    auto txs = get_block_transactions_by_height(h);
    auto internal_txs = trace_block(h);

    set_transactions(txs, internal_txs);
    insert_block_transactions(internal_txs);
  }
}

std::string eth_transaction_db::get_address_type(
    const std::string &address) {

  auto it = m_addr_and_type.find(address);
  if (it != m_addr_and_type.end()) {
    return it->second;
  }

  std::string type = ::neb::eth::get_address_type(address);
  m_addr_and_type.insert(std::make_pair(address, type));
  return type;
}

void eth_transaction_db::set_transactions(
    const std::vector<transaction_info_t> &txs,
    std::vector<transaction_info_t> &internal_txs) {

  auto it_tx = txs.begin();
  auto it_internal_tx = internal_txs.begin();

  while (it_tx != txs.end() && it_internal_tx != internal_txs.end()) {

    std::string from = it_internal_tx->template get<::neb::from>();
    std::string to = it_internal_tx->template get<::neb::to>();
    std::string type_from = get_address_type(from);
    std::string type_to = get_address_type(to);

    std::string timestamp = it_tx->template get<::neb::timestamp>();
    std::string gas_price = it_tx->template get<::neb::gas_price>();
    it_internal_tx->template set<::neb::timestamp, ::neb::gas_price,
                                 ::neb::type_from, ::neb::type_to>(
        timestamp, gas_price, type_from, type_to);

    std::string tx_hash = it_tx->template get<::neb::hash>();
    std::string internal_tx_hash = it_internal_tx->template get<::neb::hash>();

    it_internal_tx++;
    if (it_internal_tx == internal_txs.end()) {
      return;
    }

    std::string next_internal_tx_hash =
        it_internal_tx->template get<::neb::hash>();

    if (next_internal_tx_hash.compare("null") != 0 &&
        next_internal_tx_hash.compare(tx_hash) != 0) {
      it_tx++;
    }
  }
}

void eth_transaction_db::clean_transaction_db() {
  aql_query("for tx in transaction remove tx in transaction");
}

} // namespace eth
} // namespace neb
