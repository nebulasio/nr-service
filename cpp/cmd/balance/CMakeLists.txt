
add_executable(balance_writer balance_writer.cpp)
target_link_libraries(balance_writer ${common_libs} gflags)

add_executable(balance_week_writer balance_week_writer.cpp)
target_link_libraries(balance_week_writer ${common_libs} gflags)

