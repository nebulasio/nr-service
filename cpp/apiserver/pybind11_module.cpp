
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "api/account_apiserver.h"
#include "api/nr_apiserver.h"
#include "api/transaction_apiserver.h"
#include "apiserver.h"

PYBIND11_MODULE(nebserver, m) {
  google::InitGoogleLogging("apinebserver");
  FLAGS_log_dir = std::getenv("GLOG_DIR");

  pybind11::class_<apiserver>(m, "apiserver")
      .def(pybind11::init<const std::string &>())
      .def("on_api_example", &apiserver::on_api_example);

  pybind11::class_<transaction_apiserver<neb::nebulas_db>>(
      m, "nebulas_transaction_apiserver")
      .def(pybind11::init<const std::string &>())
      .def("on_api_transaction",
           &transaction_apiserver<neb::nebulas_db>::on_api_transaction);

  pybind11::class_<account_apiserver<neb::nebulas_db>>(
      m, "nebulas_account_apiserver")
      .def(pybind11::init<const std::string &>())
      .def("on_api_account",
           &account_apiserver<neb::nebulas_db>::on_api_account);

  pybind11::class_<nr_apiserver<neb::nebulas_db>>(m, "nebulas_nr_apiserver")
      .def(pybind11::init<const std::string &>())
      .def("on_api_nr", &nr_apiserver<neb::nebulas_db>::on_api_nr);

  pybind11::class_<transaction_apiserver<neb::eth_db>>(
      m, "eth_transaction_apiserver")
      .def(pybind11::init<const std::string &>())
      .def("on_api_transaction",
           &transaction_apiserver<neb::eth_db>::on_api_transaction);

  pybind11::class_<account_apiserver<neb::eth_db>>(m, "eth_account_apiserver")
      .def(pybind11::init<const std::string &>())
      .def("on_api_account", &account_apiserver<neb::eth_db>::on_api_account);

  pybind11::class_<nr_apiserver<neb::eth_db>>(m, "eth_nr_apiserver")
      .def(pybind11::init<const std::string &>())
      .def("on_api_nr", &nr_apiserver<neb::eth_db>::on_api_nr);
}
