include_directories(${PROJECT_SOURCE_DIR}/apiserver)

set(lib_src
  apiserver.cpp
  api/transaction_apiserver.cpp
  api/account_apiserver.cpp
  api/nr_apiserver.cpp)

pybind11_add_module(nebserver
  pybind11_module.cpp
  ${lib_src}
  )

target_link_libraries(nebserver PRIVATE ${common_libs} gflags)

add_executable(transaction_server ${lib_src} test/transaction_server.cpp)
target_link_libraries(transaction_server ${common_libs})
