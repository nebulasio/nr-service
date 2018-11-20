
add_executable(eth_transaction_writer eth_transaction_writer.cpp)
target_link_libraries(eth_transaction_writer ${common_libs} gflags)

add_executable(transaction_writer transaction_writer.cpp)
target_link_libraries(transaction_writer ${common_libs} gflags)

