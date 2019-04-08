
add_executable(balance_writer balance_writer.cpp)
target_link_libraries(balance_writer ${common_libs} gflags)

add_executable(balance_week balance_week.cpp)
target_link_libraries(balance_week ${common_libs} gflags)

