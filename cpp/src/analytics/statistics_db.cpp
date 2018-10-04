#include "statistics_db.h"

namespace neb {
namespace nebulas {

statistics_db::statistics_db() {}
statistics_db::statistics_db(const std::string &url, const std::string &usrname,
                             const std::string &passwd,
                             const std::string &dbname)
    : db<::neb::nebulas_db, statistics_db_info_setter>(url, usrname, passwd,
                                                       dbname) {}

void statistics_db::insert_statistics(const statistics_info_t &info) {

  auto request = ::arangodb::fuerte::createRequest(
      ::arangodb::fuerte::RestVerb::Post,
      "/_db/" + this->m_dbname + "/_api/document/statistics");

  VPackBuilder builder;
  builder.openObject();

  std::string date = info.template get<::neb::nebulas::date>();
  builder.add("_key", VPackValue(date));
  builder.add("date", VPackValue(date));
  builder.add("contract_nums",
              VPackValue(info.template get<::neb::nebulas::contract_nums>()));
  builder.add(
      "active_contract_nums",
      VPackValue(info.template get<::neb::nebulas::active_contract_nums>()));
  builder.add(
      "active_account_nums",
      VPackValue(info.template get<::neb::nebulas::active_account_nums>()));

  builder.close();
  request->addVPack(builder.slice());
  this->m_connection_ptr->sendRequest(std::move(request));
}
} // namespace nebulas
} // namespace neb
