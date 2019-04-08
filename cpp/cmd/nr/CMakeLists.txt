
add_executable(nr nr.cpp)
target_link_libraries(nr ${common_libs} gflags)

add_executable(nr_writer nr_writer.cpp)
target_link_libraries(nr_writer ${common_libs} gflags)

add_executable(nr_graph nr_graph.cpp)
target_link_libraries(nr_graph ${common_libs} gflags)

add_executable(nr_week nr_week.cpp)
target_link_libraries(nr_week ${common_libs} gflags)

