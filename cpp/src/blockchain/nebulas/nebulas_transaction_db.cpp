#include "blockchain/nebulas/nebulas_transaction_db.h"
#include "blockchain/nebulas/nebulas_api.h"
#include "blockchain/transaction_db.h"

namespace neb {
namespace nebulas {

nebulas_transaction_db::nebulas_transaction_db() {}
nebulas_transaction_db::nebulas_transaction_db(const std::string &url,
                                               const std::string &usrname,
                                               const std::string &passwd,
                                               const std::string &dbname)
    : transaction_db<nebulas_db>(url, usrname, passwd, dbname) {}

block_height_t nebulas_transaction_db::get_max_height_from_db() {

  std::unique_ptr<::arangodb::fuerte::Response> resp_ptr = aql_query(
      "for tx in transaction sort tx.height desc limit 1 return tx.height");

  auto height_doc = resp_ptr->slices().front().get("result");
  if (height_doc.isNone() || height_doc.isEmptyArray()) {
    return 1;
  }
  return height_doc.at(0).getInt();
}

void nebulas_transaction_db::remove_transactions_this_block_height(
    block_height_t block_height) {
  const std::string aql =
      boost::str(boost::format("for tx in transaction filter tx.height==%1% "
                               "remove tx in transaction") %
                 block_height);
  aql_query(aql);
}

int64_t nebulas_transaction_db::get_max_tx_id_from_db(block_height_t height) {

  const std::string aql = boost::str(
      boost::format("for tx in transaction filter tx.height==%1% sort "
                    "to_number(tx._key) desc limit 1 return tx._key") %
      height);
  std::unique_ptr<::arangodb::fuerte::Response> resp_ptr = aql_query(aql);

  auto transaction_doc = resp_ptr->slices().front().get("result");
  if (transaction_doc.isNone() || transaction_doc.isEmptyArray()) {
    return 0;
  }
  return std::stoi(transaction_doc.at(0).copyString());
}

template <class T>
void nebulas_transaction_db::insert_document(VPackBuilder &builder_arr,
                                             const std::string &collection_name,
                                             const T &document) {
  auto f_height = [&]() -> void {
    if (document.template get<::neb::tx_id>() > 0) {
      return;
    }
    block_height_t height = document.template get<::neb::height>();
    VPackBuilder builder;
    builder.openObject();
    builder.add("_key", VPackValue(std::to_string(height)));
    builder.add("block_height", VPackValue(height));
    builder.close();
    builder_arr.add(builder.slice());
  };
  auto f_address = [&]() -> void {
    if (document.template get<::neb::tx_id>() < 0) {
      return;
    }
    std::string from = document.template get<::neb::from>();
    std::string to = document.template get<::neb::to>();
    std::vector<std::string> address({from, to});

    for (auto it = address.begin(); it != address.end(); it++) {
      VPackBuilder builder;
      builder.openObject();
      builder.add("_key", VPackValue(*it));
      builder.add("address", VPackValue(*it));
      builder.close();
      builder_arr.add(builder.slice());
    }
  };
  auto f_transaction = [&]() -> void {
    if (document.template get<::neb::tx_id>() < 0) {
      return;
    }
    auto it = &document;

    VPackBuilder builder;
    builder.openObject();
    builder.add("_key",
                VPackValue(std::to_string(it->template get<::neb::tx_id>())));
    builder.add("tx_id",
                VPackValue(std::to_string(it->template get<::neb::tx_id>())));
    builder.add("nonce", VPackValue(it->template get<::neb::nonce>()));
    builder.add("status", VPackValue(it->template get<::neb::status>()));
    builder.add("chainId", VPackValue(it->template get<::neb::chainId>()));
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
  };
  auto f_height_next = [&]() -> void {
    block_height_t block_height = document.template get<::neb::height>();
    if (block_height > 1) {
      block_height_t pre_height = block_height - 1;
      VPackBuilder builder;
      builder.openObject();
      builder.add("_key", VPackValue(std::to_string(pre_height) + '-' +
                                     std::to_string(block_height)));
      builder.add("_from", VPackValue("height/" + std::to_string(pre_height)));
      builder.add("_to", VPackValue("height/" + std::to_string(block_height)));
      builder.close();
      builder_arr.add(builder.slice());
    }
  };
  auto f_height_txs = [&]() -> void {
    if (document.template get<::neb::tx_id>() < 0) {
      return;
    }
    block_height_t height = document.template get<::neb::height>();
    VPackBuilder builder;
    builder.openObject();
    int64_t tx_id = document.template get<::neb::tx_id>();
    builder.add("_key", VPackValue(std::to_string(tx_id)));
    builder.add("_from", VPackValue("height/" + std::to_string(height)));
    builder.add("_to", VPackValue("transaction/" + std::to_string(tx_id)));
    builder.close();
    builder_arr.add(builder.slice());
  };
  auto f_from_txs = [&]() -> void {
    if (document.template get<::neb::tx_id>() < 0) {
      return;
    }
    int64_t tx_id = document.template get<::neb::tx_id>();
    std::string from = document.template get<::neb::from>();
    VPackBuilder builder;
    builder.openObject();
    builder.add("_key", VPackValue(std::to_string(tx_id)));
    builder.add("_from", VPackValue("address/" + from));
    builder.add("_to", VPackValue("transaction/" + std::to_string(tx_id)));
    builder.close();
    builder_arr.add(builder.slice());
  };
  auto f_txs_to = [&]() -> void {
    if (document.template get<::neb::tx_id>() < 0) {
      return;
    }
    int64_t tx_id = document.template get<::neb::tx_id>();
    std::string to = document.template get<::neb::to>();
    VPackBuilder builder;
    builder.openObject();
    builder.add("_key", VPackValue(std::to_string(tx_id)));
    builder.add("_from", VPackValue("transaction/" + std::to_string(tx_id)));
    builder.add("_to", VPackValue("address/" + to));
    builder.close();
    builder_arr.add(builder.slice());
  };

  std::unordered_map<std::string, std::function<void()>> name_and_func(
      {{"height", f_height},
       {"address", f_address},
       {"transaction", f_transaction},
       {"height_next", f_height_next},
       {"height_txs", f_height_txs},
       {"from_txs", f_from_txs},
       {"txs_to", f_txs_to}});
  auto it = name_and_func.find(collection_name);
  it->second();
}

template <class T>
void nebulas_transaction_db::insert_documents_to_collection(
    const std::string &collection_name, const std::vector<T> &documents) {

  auto request = ::arangodb::fuerte::createRequest(
      ::arangodb::fuerte::RestVerb::Post,
      "/_db/" + m_dbname + "/_api/document/" + collection_name);

  VPackBuilder builder_arr;
  builder_arr.openArray();

  for (auto it = documents.begin(); it != documents.end(); it++) {
    insert_document(builder_arr, collection_name, *it);
  }

  builder_arr.close();
  request->addVPack(builder_arr.slice());
  m_connection_ptr->sendRequest(std::move(request));
}

void nebulas_transaction_db::append_transaction_graph_vertex_and_edge_by_block(
    const std::vector<transaction_info_t> &rs) {
  const std::string collection_names[] = {
      "height",     "address",  "transaction", "height_next",
      "height_txs", "from_txs", "txs_to"};
  for (const std::string &collection_name : collection_names) {
    insert_documents_to_collection(collection_name, rs);
  }
}

void nebulas_transaction_db::append_transaction_to_graph() {

  block_height_t last_height = get_max_height_from_db();
  remove_transactions_this_block_height(last_height);
  block_height_t current_height = ::neb::nebulas::get_block_height();
  int64_t tx_id = get_max_tx_id_from_db(last_height - 1);

  for (int h = last_height; h < current_height; h++) {

    LOG(INFO) << h;
    std::string block_timestamp =
        ::neb::nebulas::get_block_timestamp_by_height(h);
    if (block_timestamp.compare(std::string()) == 0) {
      return;
    }
    std::vector<transaction_info_t> v =
        ::neb::nebulas::get_block_transactions_by_height(h, block_timestamp);

    std::vector<transaction_info_t> rs;
    for (size_t i = 0; i < v.size(); ++i) {
      v[i].template set<::neb::tx_id>(++tx_id);
      rs.push_back(v[i]);
      int32_t tx_status = v[i].template get<::neb::status>();

      std::vector<transaction_info_t> events =
          ::neb::nebulas::get_transaction_events(v[i], block_timestamp,
                                                 tx_status);
      for (auto it = events.begin(); it != events.end(); it++) {
        it->template set<::neb::tx_id>(++tx_id);
        rs.push_back(*it);
      }
    }

    transaction_info_t info;
    info.set<::neb::tx_id, ::neb::height>(-1, h);
    rs.push_back(info);
    LOG(INFO) << "parsing block done, insert documents...";
    append_transaction_graph_vertex_and_edge_by_block(rs);
  }
}

} // end namespace nebulas
} // end namespace neb
