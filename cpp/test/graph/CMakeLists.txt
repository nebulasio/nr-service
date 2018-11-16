add_executable(test_graph main.cpp
  gtest_graph.cpp
  )
target_link_libraries(test_graph gtest ${common_libs})

