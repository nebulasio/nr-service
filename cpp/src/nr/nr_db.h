#pragma once

#include "blockchain/account_db.h"
#include "sql/table.h"

namespace neb {

define_nt(date, std::string, "date");
define_nt(median, double, "median");
define_nt(weight, double, "weight");
define_nt(score, double, "score");
define_nt(in_degree, int32_t, "in_degree");
define_nt(out_degree, int32_t, "out_degree");
define_nt(degrees, int32_t, "degrees");
define_nt(in_val, double, "in_val");
define_nt(out_val, double, "out_val");
define_nt(in_outs, double, "in_outs");

typedef ntarray<date, address, median, weight, score, in_degree, out_degree,
                degrees, in_val, out_val, in_outs>
    nr_table_t;
typedef typename nr_table_t::row_type nr_info_t;

class nr_db_interface {
public:
  virtual void insert_date_nrs(const std::vector<nr_info_t> &infos) = 0;
  virtual std::shared_ptr<std::vector<nr_info_t>>
  read_nr_by_date(const std::string &date) = 0;
  virtual std::string nr_infos_to_string(const std::vector<nr_info_t> &rs) = 0;
};

struct nr_db_info_setter {
  typedef nr_info_t info_type;

  static void set_info(nr_info_t &info, const VPackSlice &slice,
                       const std::string &key) {
    if (key.compare("date") == 0) {
      info.template set<::neb::date>(slice.copyString());
    }
    if (key.compare("address") == 0) {
      info.template set<::neb::address>(slice.copyString());
    }
    if (key.compare("median") == 0) {
      info.template set<::neb::median>(slice.isDouble() ? slice.getDouble()
                                                        : slice.getInt());
    }
    if (key.compare("weight") == 0) {
      info.template set<::neb::weight>(slice.isDouble() ? slice.getDouble()
                                                        : slice.getInt());
    }
    if (key.compare("score") == 0) {
      info.template set<::neb::score>(slice.isDouble() ? slice.getDouble()
                                                       : slice.getInt());
    }
    if (key.compare("in_degree") == 0) {
      info.template set<::neb::in_degree>(slice.getInt());
    }
    if (key.compare("out_degree") == 0) {
      info.template set<::neb::out_degree>(slice.getInt());
    }
    if (key.compare("degrees") == 0) {
      info.template set<::neb::degrees>(slice.getInt());
    }
    if (key.compare("in_val") == 0) {
      info.template set<::neb::in_val>(slice.isDouble() ? slice.getDouble()
                                                        : slice.getInt());
    }
    if (key.compare("out_val") == 0) {
      info.template set<::neb::out_val>(slice.isDouble() ? slice.getDouble()
                                                         : slice.getInt());
    }
    if (key.compare("in_outs") == 0) {
      info.template set<::neb::in_outs>(slice.isDouble() ? slice.getDouble()
                                                         : slice.getInt());
    }
  }
};

template <typename DB>
class nr_db : public db<DB, nr_db_info_setter>, public nr_db_interface {
public:
  typedef db<DB, nr_db_info_setter> base_db_t;
  nr_db() {}
  nr_db(const std::string &url, const std::string &usrname,
        const std::string &passwd, const std::string &dbname)
      : db<DB, nr_db_info_setter>(url, usrname, passwd, dbname) {}

  virtual void insert_date_nrs(const std::vector<nr_info_t> &infos) {

    auto request = ::arangodb::fuerte::createRequest(
        ::arangodb::fuerte::RestVerb::Post,
        "/_db/" + this->m_dbname + "/_api/document/nr");

    VPackBuilder builder_arr;
    builder_arr.openArray();

    uint32_t payload_size = 1 << 9;

    for (size_t i = 0; i < infos.size(); i++) {

      if (i > 0 && i % payload_size == 0) {
        auto request = ::arangodb::fuerte::createRequest(
            ::arangodb::fuerte::RestVerb::Post,
            "/_db/" + this->m_dbname + "/_api/document/nr");

        // send request with array length payload_size
        builder_arr.close();
        request->addVPack(builder_arr.slice());
        this->m_connection_ptr->sendRequest(std::move(request));
        // LOG(INFO) << (i / payload_size) << ',' << (infos.size() /
        // payload_size);

        builder_arr.clear();
        builder_arr.openArray();
      }

      insert_nr(builder_arr, infos[i]);
    }

    // for (auto info : infos) {
    // insert_nr(builder_arr, info);
    //}

    builder_arr.close();
    request->addVPack(builder_arr.slice());
    this->m_connection_ptr->sendRequest(std::move(request));
  }

  virtual std::shared_ptr<std::vector<nr_info_t>>
  read_nr_by_date(const std::string &date) {
    const std::string aql = boost::str(
        boost::format("for item in nr filter item.date=='%1%' return item") %
        date);
    return this->aql_query_with_batch(aql);
    // auto resp_ptr = this->aql_query(aql);

    // std::vector<nr_info_t> rs;
    // base_db_t::parse_from_response(std::move(resp_ptr), rs);
    // return std::make_shared<std::vector<nr_info_t>>(rs);
  }

  virtual std::string nr_infos_to_string(const std::vector<nr_info_t> &rs) {
    boost::property_tree::ptree root;
    boost::property_tree::ptree arr;

    for (auto it = rs.begin(); it != rs.end(); it++) {
      const nr_info_t &info = *it;
      boost::property_tree::ptree p;
      convert_nr_info_to_ptree(info, p);
      arr.push_back(std::make_pair(std::string(), p));
    }
    root.add_child("nr", arr);
    return base_db_t::ptree_to_string(root);
  }

private:
  static void insert_nr(VPackBuilder &builder_arr, const nr_info_t &info) {
    std::string date = info.template get<::neb::date>();
    std::string address = info.template get<::neb::address>();

    auto it = &info;

    VPackBuilder builder;
    builder.openObject();
    builder.add("_key", VPackValue(date + '-' + address));
    builder.add("date", VPackValue(date));
    builder.add("address", VPackValue(address));
    builder.add("median", VPackValue(it->template get<::neb::median>()));
    builder.add("weight", VPackValue(it->template get<::neb::weight>()));
    builder.add("score", VPackValue(it->template get<::neb::score>()));
    builder.add("in_degree", VPackValue(it->template get<::neb::in_degree>()));
    builder.add("out_degree",
                VPackValue(it->template get<::neb::out_degree>()));
    builder.add("degrees", VPackValue(it->template get<::neb::degrees>()));
    builder.add("in_val", VPackValue(it->template get<::neb::in_val>()));
    builder.add("out_val", VPackValue(it->template get<::neb::out_val>()));
    builder.add("in_outs", VPackValue(it->template get<::neb::in_outs>()));
    builder.close();
    builder_arr.add(builder.slice());
  }

  static void convert_nr_info_to_ptree(const nr_info_t &info,
                                       boost::property_tree::ptree &p) {
    std::string date = info.template get<::neb::date>();
    std::string address = info.template get<::neb::address>();
    double median = info.template get<::neb::median>();
    double weight = info.template get<::neb::weight>();
    double score = info.template get<::neb::score>();
    int32_t in_degree = info.template get<::neb::in_degree>();
    int32_t out_degree = info.template get<::neb::out_degree>();
    int32_t degrees = info.template get<::neb::degrees>();
    double in_val = info.template get<::neb::in_val>();
    double out_val = info.template get<::neb::out_val>();
    double in_outs = info.template get<::neb::in_outs>();

    std::unordered_map<std::string, std::string> kv_pair(
        {{"date", date},
         {"address", address},
         {"median", std::to_string(median)},
         {"weight", std::to_string(weight)},
         {"score", std::to_string(score)},
         {"in_degree", std::to_string(in_degree)},
         {"out_degree", std::to_string(out_degree)},
         {"degrees", std::to_string(degrees)},
         {"in_val", std::to_string(in_val)},
         {"out_val", std::to_string(out_val)},
         {"in_outs", std::to_string(in_outs)}});

    for (auto &ele : kv_pair) {
      p.put(ele.first, ele.second);
    }
  }
};
} // namespace neb
