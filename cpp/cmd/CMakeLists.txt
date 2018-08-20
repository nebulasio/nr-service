
add_executable(main main.cpp)
target_link_libraries(main ${common_libs} gflags)

add_executable(ntobject ntobject.cpp)
target_link_libraries(ntobject ${common_libs} gflags)

