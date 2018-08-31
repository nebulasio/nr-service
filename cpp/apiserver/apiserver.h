#pragma once

#include "utils.h"

class apiserver {
public:
  apiserver(const std::string &appname);

  void
  on_api_example(const std::unordered_map<std::string, std::string> &params);

protected:
  std::string m_appname;
}; // end class apiserver
