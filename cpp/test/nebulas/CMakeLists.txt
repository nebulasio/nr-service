add_executable(test_nebulas main.cpp
  gtest_nebulas_account_db.cpp
  gtest_nebulas_transaction_db.cpp
  )
target_link_libraries(test_nebulas gtest ${common_libs})

