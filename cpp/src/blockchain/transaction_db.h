#pragma once

#include "common.h"
#include "sql/ntobject.h"
#include "sql/table.h"

namespace neb {

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

typedef ntarray<nonce, status, chainId, from, timestamp, gas_used, tx_value,
                data, to, contract_address, hash, gas_price, tx_type, gas_limit,
                height, type_from, type_to>
    transaction_table_t;
typedef typename transaction_table_t::row_type transaction_info_t;

namespace internal {
template <typename TS> struct transaction_db_traits {};
template <> struct transaction_db_traits<nebulas_db> {
  constexpr const static char *table_name = "nebulas_transaction_db";
};
template <> struct transaction_db_traits<eth_db> {
  constexpr const static char *table_name = "eth_transaction_db";
};
} // namespace internal

class transaction_db_interface {}; // namespace neb

template <typename DB> class transaction_db : public transaction_db_interface {
public:
  transaction_db() {}
  transaction_db(const std::string &url, const std::string &usrname,
                 const std::string &passwd) {
    m_conn_builder.host(url);
    m_conn_builder.authenticationType(
        ::arangodb::fuerte::AuthenticationType::Basic);
    m_conn_builder.user(usrname);
    m_conn_builder.password(passwd);

    m_event_loop_service_ptr =
        std::shared_ptr<::arangodb::fuerte::EventLoopService>(
            new ::arangodb::fuerte::EventLoopService(1));
    m_connection_ptr = m_conn_builder.connect(*m_event_loop_service_ptr);
  }

  inline const std::shared_ptr<::arangodb::fuerte::Connection>
  transaction_db_connection_ptr() const {
    return m_connection_ptr;
  }

protected:
  ::arangodb::fuerte::ConnectionBuilder m_conn_builder;
  std::shared_ptr<::arangodb::fuerte::EventLoopService>
      m_event_loop_service_ptr;
  std::shared_ptr<::arangodb::fuerte::Connection> m_connection_ptr;

}; // end class transaction_db
} // namespace neb
