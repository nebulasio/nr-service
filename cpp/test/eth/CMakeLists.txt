add_executable(test_eth main.cpp
  gtest_eth_transaction_db.cpp
  )
target_link_libraries(test_eth gtest ${common_libs})
