
add_executable(main main.cpp)
target_link_libraries(main ${common_libs} gflags)

add_subdirectory(account)
add_subdirectory(balance)
add_subdirectory(nr)
add_subdirectory(statistics)
add_subdirectory(test)
add_subdirectory(transaction)
