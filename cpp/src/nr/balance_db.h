#pragma once

#include "blockchain/account_db.h"
#include "nr/nr_db.h"

namespace neb {

typedef ntarray<date, address, balance, account_type> balance_table_t;
typedef typename balance_table_t::row_type balance_info_t;

class balance_db_interface {
public:
  virtual void
  insert_date_balances(const std::vector<balance_info_t> &infos) = 0;
  virtual std::shared_ptr<std::vector<balance_info_t>>
  read_balance_by_date(const std::string &date) = 0;
};

struct balance_db_info_setter {
  typedef balance_info_t info_type;
  static void set_info(balance_info_t &info, const VPackSlice &slice,
                       const std::string &key) {
    if (key.compare("date") == 0) {
      info.template set<::neb::date>(slice.copyString());
    }
    if (key.compare("address") == 0) {
      info.template set<::neb::address>(slice.copyString());
    }
    if (key.compare("balance") == 0) {
      info.template set<::neb::balance>(slice.copyString());
    }
    if (key.compare("type") == 0) {
      info.template set<::neb::account_type>(slice.copyString());
    }
  }
};

template <typename DB>
class balance_db : public db<DB, balance_db_info_setter>,
                   public balance_db_interface {
public:
  typedef db<DB, balance_db_info_setter> base_db_t;
  balance_db() {}
  balance_db(const std::string &url, const std::string &usrname,
             const std::string &passwd, const std::string &dbname)
      : db<DB, balance_db_info_setter>(url, usrname, passwd, dbname) {}

  virtual void insert_date_balances(const std::vector<balance_info_t> &infos) {

    auto request = ::arangodb::fuerte::createRequest(
        ::arangodb::fuerte::RestVerb::Post,
        "/_db/" + this->m_dbname + "/_api/document/balance");

    VPackBuilder builder_arr;
    builder_arr.openArray();

    uint32_t payload_size = 1 << 9;

    for (size_t i = 0; i < infos.size(); i++) {

      if (i > 0 && i % payload_size == 0) {
        auto request = ::arangodb::fuerte::createRequest(
            ::arangodb::fuerte::RestVerb::Post,
            "/_db/" + this->m_dbname + "/_api/document/balance");

        // send request with array length payload_size
        builder_arr.close();
        request->addVPack(builder_arr.slice());
        this->m_connection_ptr->sendRequest(std::move(request));

        builder_arr.clear();
        builder_arr.openArray();
      }

      insert_balance(builder_arr, infos[i]);
    }

    builder_arr.close();
    request->addVPack(builder_arr.slice());
    this->m_connection_ptr->sendRequest(std::move(request));
  }

  virtual std::shared_ptr<std::vector<balance_info_t>>
  read_balance_by_date(const std::string &date) {
    const std::string aql = boost::str(
        boost::format(
            "for item in balance filter item.date=='%1%' return item") %
        date);
    return this->aql_query_with_batch(aql);
    // auto resp_ptr = this->aql_query(aql);

    // std::vector<balance_info_t> rs;
    // base_db_t::parse_from_response(std::move(resp_ptr), rs);
    // return std::make_shared<std::vector<balance_info_t>>(rs);
  }

private:
  static void insert_balance(VPackBuilder &builder_arr,
                             const balance_info_t &info) {
    std::string date = info.template get<::neb::date>();
    std::string address = info.template get<::neb::address>();

    auto it = &info;

    VPackBuilder builder;
    builder.openObject();
    builder.add("_key", VPackValue(date + '-' + address));
    builder.add("date", VPackValue(date));
    builder.add("address", VPackValue(address));
    builder.add("balance", VPackValue(it->template get<::neb::balance>()));
    builder.add("type", VPackValue(it->template get<::neb::account_type>()));
    builder.close();
    builder_arr.add(builder.slice());
  }
};

} // namespace neb
