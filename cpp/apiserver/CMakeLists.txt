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

add_executable(test_lru_cache test/lru_cache.cpp)
target_link_libraries(test_lru_cache ${common_libs} gflags)


add_executable(test_transaction_server ${lib_src} test/test_transaction_server.cpp)
target_link_libraries(test_transaction_server ${common_libs})
