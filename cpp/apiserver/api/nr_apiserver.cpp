#include "nr_apiserver.h"
#include "err/err_def.h"

nr_apiserver::nr_apiserver(const std::string &appname) : apiserver(appname) {
  m_cache_ptr = std::make_unique<nr_cache_t>();
  m_nr_ptr = std::make_shared<nebulas_nr_db_t>(
      nebulas_nr_db_t(std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
                      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB")));
}

std::string nr_apiserver::on_api_nr(
    const std::unordered_map<std::string, std::string> &params) {

  LOG(INFO) << "on api nr, request params";
  for (auto it = params.begin(); it != params.end(); it++) {
    LOG(INFO) << it->first << ',' << it->second;
  }

  if (params.find("date") == params.end()) {
    LOG(WARNING) << "params not matched, no param \'date\'";
    return err_code_params_not_matched;
  }
  std::string date = params.find("date")->second;
  if (!neb::is_number(date)) {
    LOG(WARNING)
        << "params value invalid, date value contains unexpected character";
    return err_code_params_value_invalid;
  }

  nr_cache_t &cache = *m_cache_ptr;
  std::vector<neb::nr_info_t> rs;

  if (!cache.get(date, rs)) {
    LOG(INFO) << "nr cache missed, reading from db";
    set_nr_cache(cache, date);

    if (!cache.get(date, rs)) {
      LOG(INFO) << "nr db missed";
    }
  }
  return m_nr_ptr->to_string(rs);
}

void nr_apiserver::set_nr_cache(nr_cache_t &cache, const std::string &date) {
  std::vector<neb::nr_info_t> rs = m_nr_ptr->read_nr_by_date(date);
  LOG(INFO) << "read nr by date, size: " << rs.size();
  cache.set(date, rs);
  return;
}
