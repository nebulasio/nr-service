
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

