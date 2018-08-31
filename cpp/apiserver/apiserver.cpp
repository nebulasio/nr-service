#include "apiserver.h"
#include "utils.h"

apiserver::apiserver(const std::string &appname) : m_appname(appname) {}

void apiserver::on_api_example(
    const std::unordered_map<std::string, std::string> &params) {
  std::cout << "on_api_example" << std::endl;
}

