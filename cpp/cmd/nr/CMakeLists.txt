
add_executable(nr_auto nr_auto.cpp)
target_link_libraries(nr_auto ${common_libs} gflags)

add_executable(nr_writer nr_writer.cpp)
target_link_libraries(nr_writer ${common_libs} gflags)

add_executable(nr_graph nr_graph.cpp)
target_link_libraries(nr_graph ${common_libs} gflags)

add_executable(nr_week_writer nr_week_writer.cpp)
target_link_libraries(nr_week_writer ${common_libs} gflags)

add_executable(nr_week_auto nr_week_auto.cpp)
target_link_libraries(nr_week_auto ${common_libs} gflags)

