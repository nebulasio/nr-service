#pragma once

#include <boost/date_time/gregorian/conversion.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <gflags/gflags.h>

#define STRITEM(v) #v
#define STR(v) STRITEM(v)

#include "common.h"

namespace neb {

class utils {};

class file_utils : public utils {
public:
  static std::string get_stdout_from_command(std::string &cmd);
  static std::vector<std::string> split_by_comma(const std::string &str,
                                                 char comma);
  static void read_lines_from_file(const std::string &file,
                                   std::vector<std::string> &lines);
};

class time_utils : public utils {
public:
  static time_t get_universal_timestamp();
  static std::string time_t_to_date(time_t t);

private:
  static std::string fill_prefix_zero(int number, int digit);
  static std::string
  replace_month_str_to_month_of_year(std::string &simple_str);

  static std::string
  ptime_str_local_to_ptime_str_utc(const std::string &ptime_str_local);
  static boost::posix_time::ptime
  ptime_str_to_ptime(const std::string &ptime_str);
  static time_t ptime_str_to_time_t(const std::string &ptime_str);
};

class string_utils : public utils {
public:
  static std::string to_dec(const std::string &hex_str);
  static std::string to_hex(const std::string &dec_str);
  static bool is_number(const std::string &s);
};

} // namespace neb
