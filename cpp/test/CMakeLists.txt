add_executable(nebulas_test main.cpp
  gtest_nebulas_account_db.cpp
  gtest_nebulas_transaction_db.cpp
  )
target_link_libraries(nebulas_test gtest ${common_libs})

