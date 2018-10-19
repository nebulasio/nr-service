add_executable(eth_test main.cpp
  gtest_eth_transaction_db.cpp
  )
target_link_libraries(eth_test gtest ${common_libs})
