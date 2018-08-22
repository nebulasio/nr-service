#pragma once

#include "common.h"
#include "sql/ntobject.h"
#include "sql/table.h"

#include <fuerte/connection.h>
#include <fuerte/requests.h>
#include <velocypack/velocypack-aliases.h>

namespace neb {

define_nt(tx_id, int64_t, "tx_id");
define_nt(nonce, std::string, "nonce");
define_nt(status, int32_t, "status");
define_nt(chainId, int32_t, "chainId");
define_nt(from, std::string, "from");
define_nt(timestamp, std::string, "timestamp");
define_nt(gas_used, std::string, "gas_used");
define_nt(tx_value, std::string, "value");
define_nt(data, std::string, "data");
define_nt(to, std::string, "to");
define_nt(contract_address, std::string, "contract_address");
define_nt(hash, std::string, "hash");
define_nt(gas_price, std::string, "gas_price");
define_nt(tx_type, std::string, "type");
define_nt(gas_limit, std::string, "gas_limit");
define_nt(height, int64_t, "height");
define_nt(type_from, std::string, "type_from");
define_nt(type_to, std::string, "type_to");

typedef ntarray<tx_id, nonce, status, chainId, from, timestamp, gas_used,
                tx_value, data, to, contract_address, hash, gas_price, tx_type,
                gas_limit, height, type_from, type_to>
    transaction_table_t;
typedef typename transaction_table_t::row_type transaction_info_t;

class transaction_db_interface {};

template <typename DB> class transaction_db : public transaction_db_interface {
public:
  transaction_db() {}
  transaction_db(const std::string &url, const std::string &usrname,
                 const std::string &passwd, const std::string &dbname)
      : m_dbname(dbname) {
    ::arangodb::fuerte::ConnectionBuilder conn_builder;
    conn_builder.host(url);
    conn_builder.authenticationType(
        ::arangodb::fuerte::AuthenticationType::Basic);
    conn_builder.user(usrname);
    conn_builder.password(passwd);

    m_event_loop_service_ptr =
        std::shared_ptr<::arangodb::fuerte::EventLoopService>(
            new ::arangodb::fuerte::EventLoopService(1));
    m_connection_ptr = conn_builder.connect(*m_event_loop_service_ptr);
  }

  inline const std::shared_ptr<::arangodb::fuerte::Connection>
  transaction_db_connection_ptr() const {
    return m_connection_ptr;
  }

public:
  std::unique_ptr<::arangodb::fuerte::Response>
  aql_query(const std::string &aql) {
    auto request =
        ::arangodb::fuerte::createRequest(::arangodb::fuerte::RestVerb::Post,
                                          "/_db/" + m_dbname + "/_api/cursor");
    VPackBuilder builder;
    builder.openObject();
    builder.add("query", VPackValue(aql));
    builder.close();
    request->addVPack(builder.slice());
    return m_connection_ptr->sendRequest(std::move(request));
  }

protected:
  std::shared_ptr<::arangodb::fuerte::EventLoopService>
      m_event_loop_service_ptr;
  std::shared_ptr<::arangodb::fuerte::Connection> m_connection_ptr;
  std::string m_dbname;

}; // end class transaction_db
} // namespace neb
