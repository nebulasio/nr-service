#pragma once

#define DEF_ERROR_CODE(a, b)                                                   \
  const static std::string err_code_##a =                                      \
      std::string("{\"err\": \"") + std::string(b) + std::string("\"}\n");

#include "err_msg.h"

#undef DEF_ERROR_CODE

