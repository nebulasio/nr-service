
add_library(tga_engine
  graph/graph.cpp
  graph/algo.cpp
  utils/base58.cpp
  utils/base64.cpp
  utils/util.cpp
  blockchain/nebulas/nebulas_currency.cpp
  blockchain/nebulas/nebulas_transaction_db.cpp
  blockchain/nebulas/nebulas_api.cpp
  blockchain/nebulas/nebulas_account_db.cpp
  nr/nebulas_rank.cpp)
