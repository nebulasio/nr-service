#include "blockchain/nebulas/nebulas_currency.h"

neb::nebulas::nas operator"" _nas(long double v) {
  return neb::nebulas::nas(v);
}
neb::nebulas::nas operator"" _nas(const char *s) {
  return neb::nebulas::nas(std::atoi(s));
}

neb::nebulas::wei operator"" _wei(long double v) {
  return neb::nebulas::wei(v);
}
neb::nebulas::wei operator"" _wei(const char *s) {
  return neb::nebulas::wei(std::atoi(s));
}

std::ostream &operator<<(std::ostream &os, const neb::nebulas::nas &obj) {
  os << obj.value() << "nas";
  return os;
}

std::ostream &operator<<(std::ostream &os, const neb::nebulas::wei &obj) {
  os << obj.value() << "wei";
  return os;
}
