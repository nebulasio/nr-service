#pragma once
#include "blockchain/transaction_db.h"
#include <thread>

namespace neb {
namespace nebulas {

class nebulas_transaction_db : public transaction_db<nebulas_db> {

public:
  nebulas_transaction_db();
  nebulas_transaction_db(const std::string &url, const std::string &usrname,
                         const std::string &passwd);

  void append_transaction_to_db();

}; // end class neubulas_transaction_db

} // end namespace nebulas
} // end namespace neb
