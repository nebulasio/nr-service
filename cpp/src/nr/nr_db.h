#pragma once

#include "blockchain/account_db.h"
#include "sql/mysql.hpp"
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
  virtual void insert_document_to_collection(const nr_info_t &info) = 0;
};

template <typename DB> class nr_db : public db<DB>, public nr_db_interface {
public:
  nr_db() {}
  nr_db(const std::string &url, const std::string &usrname,
        const std::string &passwd, const std::string &dbname)
      : db<DB>(url, usrname, passwd, dbname) {}

  virtual void insert_document_to_collection(const nr_info_t &info) {
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

    const std::string aql = boost::str(
        boost::format(
            "upsert {_key:'%1%'} "
            "insert {_key:'%1%', date:'%2%', address:'%3%', median:%4%, "
            "weight:%5%, score:%6%, in_degree:%7%, out_degree:%8%, "
            "degrees:%9%, in_val:'%10%', out_val:'%11%', in_outs:'%12%'} "
            "update {date:'%2%', address:'%3%', median:%4%, weight:%5%, "
            "score:%6%, in_degree:%7%, out_degree:%8%, degrees:%9%, "
            "in_val:%10%, out_val:%11%, in_outs:%12%} in account") %
        (date + '/' + address) % date % address % median % weight % score %
        in_degree % out_degree % degrees % in_val % out_val % in_outs);
    this->aql_query(aql);
  }
};
} // namespace neb
