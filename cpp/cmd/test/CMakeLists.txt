
add_executable(ntobject ntobject.cpp)
target_link_libraries(ntobject ${common_libs} gflags)

add_executable(base58 base58.cpp)
target_link_libraries(base58 ${common_libs} gflags)

add_executable(eth_api eth_api.cpp)
target_link_libraries(eth_api ${common_libs} gflags)

add_executable(lru_cache lru_cache.cpp)
target_link_libraries(lru_cache ${common_libs} gflags)

add_executable(monitor exchanges_monitor.cpp)
target_link_libraries(monitor ${common_libs} gflags)

add_executable(graph graph.cpp)
target_link_libraries(graph ${common_libs} gflags)

add_executable(blockchain blockchain.cpp)
target_link_libraries(blockchain ${common_libs} gflags)

