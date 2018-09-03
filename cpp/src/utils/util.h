#pragma once

#include <boost/date_time/gregorian/conversion.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <gflags/gflags.h>

#define STRITEM(v) #v
#define STR(v) STRITEM(v)

#include "common.h"

namespace neb {

std::string get_stdout_from_command(std::string &cmd);

std::vector<std::string> split_by_comma(const std::string &str, char comma);

void read_lines_from_file(const std::string &file,
                          std::vector<std::string> &lines);

bool is_number(const std::string &s);

time_t get_universal_timestamp();
} // namespace neb
