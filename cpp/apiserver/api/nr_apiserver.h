#pragma once

#include "apiserver.h"
#include "cache/lru_cache.h"
#include "nr.h"

typedef neb::lru_cache<std::string, std::vector<neb::nr_info_t>> nr_cache_t;
typedef neb::nr_db<neb::nebulas_db> nebulas_nr_db_t;
typedef std::shared_ptr<nebulas_nr_db_t> nebulas_nr_db_ptr_t;

class nr_apiserver : public apiserver {
public:
  nr_apiserver(const std::string &appname, size_t cache_size);

  std::string
  on_api_nr(const std::unordered_map<std::string, std::string> &params);

private:
  void set_nr_cache(nr_cache_t &cache, const std::string &date);

protected:
  std::unique_ptr<nr_cache_t> m_cache_ptr;
  nebulas_nr_db_ptr_t m_nr_ptr;
};

