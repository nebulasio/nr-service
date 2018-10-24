#pragma once

#include "apiserver.h"
#include "err/err_def.h"
#include "nr.h"

typedef neb::lru_cache<std::string,
                       std::shared_ptr<std::vector<neb::nr_info_t>>>
    nr_cache_t;
typedef neb::nr_db_interface nr_db_t;
typedef std::shared_ptr<nr_db_t> nr_db_ptr_t;

template <typename TS> struct nr_apiserver_traits {};
template <> struct nr_apiserver_traits<neb::nebulas_db> {
  static nr_db_ptr_t get_nr_db_ptr() {
    typedef neb::nr_db<neb::nebulas_db> nebulas_nr_db_t;
    return std::make_shared<nebulas_nr_db_t>(
        std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
        std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  }
};
template <> struct nr_apiserver_traits<neb::eth_db> {
  static nr_db_ptr_t get_nr_db_ptr() {
    typedef neb::nr_db<neb::eth_db> eth_nr_db_t;
    return std::make_shared<eth_nr_db_t>(
        std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
        std::getenv("DB_PASSWORD"), std::getenv("ETH_DB"));
  }
};

template <class DB> class nr_apiserver : public apiserver {
public:
  nr_apiserver(const std::string &appname) : apiserver(appname) {
    m_cache_ptr = std::unique_ptr<nr_cache_t>(new nr_cache_t());
    m_nr_ptr = nr_apiserver_traits<DB>::get_nr_db_ptr();
  }

  std::string
  on_api_nr(const std::unordered_map<std::string, std::string> &params) {

    LOG(INFO) << "on api nr, request params";
    for (auto it = params.begin(); it != params.end(); it++) {
      LOG(INFO) << it->first << ',' << it->second;
    }

    if (params.find("date") == params.end()) {
      LOG(WARNING) << "params not matched, no param \'date\'";
      return err_code_params_not_matched;
    }
    std::string date = params.find("date")->second;
    if (!neb::string_utils::is_number(date)) {
      LOG(WARNING)
          << "params value invalid, date value contains unexpected character";
      return err_code_params_value_invalid;
    }

    nr_cache_t &cache = *m_cache_ptr;
    std::shared_ptr<std::vector<neb::nr_info_t>> rs;

    if (!cache.get(date, rs)) {
      LOG(INFO) << "nr cache missed, reading from db";
      set_nr_cache(cache, date);

      if (!cache.get(date, rs)) {
        LOG(INFO) << "nr db missed";
      }
    }
    return m_nr_ptr->nr_infos_to_string(*rs);
  }

private:
  void set_nr_cache(nr_cache_t &cache, const std::string &date) {
    auto it_rs = m_nr_ptr->read_nr_by_date(date);
    auto rs = *it_rs;
    LOG(INFO) << "read nr by date, size: " << rs.size();
    cache.set(date, std::make_shared<std::vector<neb::nr_info_t>>(rs));
    return;
  }

protected:
  std::unique_ptr<nr_cache_t> m_cache_ptr;
  nr_db_ptr_t m_nr_ptr;
};

