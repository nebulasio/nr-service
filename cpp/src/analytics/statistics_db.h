#pragma once
#include "blockchain/db.h"
#include "common.h"
#include "sql/table.h"

namespace neb {
namespace nebulas {

define_nt(date, std::string, "date");
define_nt(contract_nums, int32_t, "contract_nums");
define_nt(active_account_nums, int32_t, "active_account_nums");
define_nt(active_contract_nums, int32_t, "active_contract_nums");

typedef ntarray<date, contract_nums, active_account_nums, active_contract_nums>
    statistics_table_t;

typedef typename statistics_table_t::row_type statistics_info_t;

class statistics_db_interface {
public:
  virtual void insert_statistics(const statistics_info_t &info) = 0;
};

struct statistics_db_info_setter {
  typedef statistics_info_t info_type;

  static void set_info(statistics_info_t &info, const VPackSlice &slice,
                       const std::string &key) {
    if (key.compare("date") == 0) {
      info.template set<::neb::nebulas::date>(slice.copyString());
    }
    if (key.compare("contract_nums") == 0) {
      info.template set<::neb::nebulas::contract_nums>(slice.getInt());
    }
    if (key.compare("active_contract_nums") == 0) {
      info.template set<::neb::nebulas::active_contract_nums>(slice.getInt());
    }
    if (key.compare("active_account_nums") == 0) {
      info.template set<::neb::nebulas::active_account_nums>(slice.getInt());
    }
  }
};

class statistics_db : public db<::neb::nebulas_db, statistics_db_info_setter>,
                      public statistics_db_interface {
public:
  statistics_db();
  statistics_db(const std::string &url, const std::string &usrname,
                const std::string &passwd, const std::string &dbname);

  virtual void insert_statistics(const statistics_info_t &info);
};
} // namespace nebulas
} // namespace neb
