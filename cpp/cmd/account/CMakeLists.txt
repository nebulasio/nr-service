
add_executable(account_writer account_writer.cpp)
target_link_libraries(account_writer ${common_libs} gflags)

add_executable(eth_account_writer eth_account_writer.cpp)
target_link_libraries(eth_account_writer ${common_libs} gflags)

