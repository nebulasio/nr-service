#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "api/transaction_apiserver.h"
#include "apiserver.h"

PYBIND11_MODULE(nebserver, m) {
  pybind11::class_<apiserver>(m, "apiserver")
      .def(pybind11::init<const std::string &>())
      .def("on_api_example", &apiserver::on_api_example);

  pybind11::class_<transaction_apiserver>(m, "transaction_apiserver")
      .def(pybind11::init<const std::string &, int32_t>())
      .def("on_api_transaction", &transaction_apiserver::on_api_transaction);
}
