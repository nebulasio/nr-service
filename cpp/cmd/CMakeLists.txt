
add_executable(main main.cpp)
target_link_libraries(main ${common_libs} gflags)

add_executable(ntobject ntobject.cpp)
target_link_libraries(ntobject ${common_libs} gflags)

add_executable(blockchain blockchain.cpp)
target_link_libraries(blockchain ${common_libs} gflags)

add_executable(transaction_writer transaction_writer.cpp)
target_link_libraries(transaction_writer ${common_libs} gflags)

add_executable(account_writer account_writer.cpp)
target_link_libraries(account_writer ${common_libs} gflags)

add_executable(nr nr.cpp)
target_link_libraries(nr ${common_libs} gflags)

add_executable(base58 base58.cpp)
target_link_libraries(base58 ${common_libs} gflags)

add_executable(eth_api eth_api.cpp)
target_link_libraries(eth_api ${common_libs} gflags)

add_executable(lru_cache lru_cache.cpp)
target_link_libraries(lru_cache ${common_libs} gflags)

add_executable(eth_transaction_writer eth_transaction_writer.cpp)
target_link_libraries(eth_transaction_writer ${common_libs} gflags)

