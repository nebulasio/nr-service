#pragma once

#include "common.h"
#include <pybind11/pybind11.h>

class apiserver {
public:
  apiserver(const std::string &appname);

  void
  on_api_example(const std::unordered_map<std::string, std::string> &params);

protected:
  std::string m_appname;

}; // end class apiserver

namespace py = pybind11;

PYBIND11_MODULE(nebserver, m) {
  py::class_<apiserver>(m, "apiserver")
      .def(py::init<const std::string &>())
      .def("on_api_example", &apiserver::on_api_example);
}

