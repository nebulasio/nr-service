#pragma once

#define DEF_ERROR_CODE(a, b) const static std::string err_code_##a = b;
#include "err_msg.h"
#undef DEF_ERROR_CODE

