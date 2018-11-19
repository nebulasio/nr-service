#pragma once

#include "common.h"

#include <fuerte/connection.h>
#include <fuerte/requests.h>
#include <velocypack/velocypack-aliases.h>

namespace neb {

class db_interface {};

template <typename DB, typename InfoSetter> class db : public db_interface {
public:
  db() {}
  db(const std::string &url, const std::string &usrname,
     const std::string &passwd, const std::string &dbname)
      : m_dbname(dbname) {
    ::arangodb::fuerte::ConnectionBuilder conn_builder;
    conn_builder.endpoint(url);
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
  db_connection_ptr() const {
    return m_connection_ptr;
  }

public:
  std::unique_ptr<::arangodb::fuerte::Response>
  aql_query(const std::string &aql, int32_t batch_size = 0x7fffffff,
            int32_t ttl = 600) {
    auto request =
        ::arangodb::fuerte::createRequest(::arangodb::fuerte::RestVerb::Post,
                                          "/_db/" + m_dbname + "/_api/cursor");
    VPackBuilder builder;
    builder.openObject();
    builder.add("query", VPackValue(aql));
    builder.add("batchSize", VPackValue(batch_size));
    builder.add("ttl", VPackValue(ttl));
    builder.close();
    request->addVPack(builder.slice());
    return m_connection_ptr->sendRequest(std::move(request));
  }

  std::unique_ptr<::arangodb::fuerte::Response>
  aql_query_with_batch_helper(const std::string &id) {
    auto request = ::arangodb::fuerte::createRequest(
        ::arangodb::fuerte::RestVerb::Put,
        "/_db/" + m_dbname + "/_api/cursor/" + id);
    return m_connection_ptr->sendRequest(std::move(request));
  }

  std::shared_ptr<std::vector<typename InfoSetter::info_type>>
  aql_query_with_batch(const std::string &aql, int32_t batch_size = 1 << 10) {

    std::vector<typename InfoSetter::info_type> rs;
    auto resp_ptr = aql_query(aql, batch_size);
    parse_from_response(resp_ptr, rs);

    int32_t has_more = resp_ptr->slices().front().get("hasMore").getBool();
    while (has_more) {
      LOG(INFO) << "result set size: " << rs.size();
      std::string id = resp_ptr->slices().front().get("id").copyString();
      resp_ptr = aql_query_with_batch_helper(id);
      has_more = resp_ptr->slices().front().get("hasMore").getBool();
      parse_from_response(resp_ptr, rs);
    }
    return std::make_shared<std::vector<typename InfoSetter::info_type>>(rs);
  }

protected:
  static void parse_from_response(
      const std::unique_ptr<::arangodb::fuerte::Response> &resp_ptr,
      std::vector<typename InfoSetter::info_type> &rs) {

    auto documents = resp_ptr->slices().front().get("result");
    if (documents.isNone() || documents.isEmptyArray()) {
      return;
    }

    for (size_t i = 0; i < documents.length(); i++) {
      auto doc = documents.at(i);
      typename InfoSetter::info_type info;
      for (size_t j = 0; j < doc.length(); j++) {
        std::string key = doc.keyAt(j).copyString();
        InfoSetter::set_info(info, doc.valueAt(j), key);
      }
      rs.push_back(info);
    }
    return;
  }

  static std::string ptree_to_string(const boost::property_tree::ptree &root) {
    std::stringstream ss;
    write_json(ss, root, false);
    LOG(INFO) << "write json: " << ss.str();
    return ss.str();
  }

protected:
  std::shared_ptr<::arangodb::fuerte::EventLoopService>
      m_event_loop_service_ptr;
  std::shared_ptr<::arangodb::fuerte::Connection> m_connection_ptr;
  std::string m_dbname;

}; // end class db
} // namespace neb
