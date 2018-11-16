#include "blockchain.h"
#include "graph.h"
#include "gtest_common.h"
#include "utils.h"
#include <gtest/gtest.h>

neb::transaction_graph_ptr_t tgp_raw =
    std::make_shared<neb::transaction_graph>();
neb::transaction_graph_ptr_t tgp = std::make_shared<neb::transaction_graph>();

template <class TransInfo>
neb::transaction_graph_ptr_t
build_graph_from_transactions(const std::vector<TransInfo> &trans) {
  neb::transaction_graph_ptr_t ret = std::make_shared<neb::transaction_graph>();

  for (auto ite = trans.begin(); ite != trans.end(); ite++) {
    std::string from = ite->template get<::neb::from>();
    std::string to = ite->template get<::neb::to>();
    std::string tx_value = ite->template get<::neb::tx_value>();
    double value = std::stod(tx_value);
    double timestamp = std::stod(ite->template get<::neb::timestamp>());
    ret->add_edge(from, to, value, timestamp);
  }
  return ret;
}

TEST(test_graph, test_read_write_graphviz) {

  transaction_ptr_t transaction_db_ptr = std::make_shared<transaction_db_t>(
      std::getenv("DB_URL"), std::getenv("DB_USER_NAME"),
      std::getenv("DB_PASSWORD"), std::getenv("NEBULAS_DB"));
  auto txs_ptr =
      transaction_db_ptr->read_inter_transaction_from_db_with_duration(1000000,
                                                                       1205000);

  tgp_raw = build_graph_from_transactions(*txs_ptr);
  tgp_raw->write_to_graphviz("xx.dot");

  auto tmp = std::make_shared<neb::transaction_graph>();
  tmp->read_from_graphviz("xx.dot");
  tmp->write_to_graphviz("yy.dot");

  tgp = std::make_shared<neb::transaction_graph>();
  tgp->read_from_graphviz("yy.dot");
}

TEST(test_graph, test_get_in_out_vals) {
  auto in_out_vals_raw =
      neb::graph_algo::get_in_out_vals(tgp_raw->internal_graph());
  auto in_out_vals = neb::graph_algo::get_in_out_vals(tgp->internal_graph());

  EXPECT_EQ(in_out_vals_raw->size(), in_out_vals->size());
  for (auto &it_raw : *in_out_vals_raw) {
    auto it = in_out_vals->find(it_raw.first);
    EXPECT_TRUE(it != in_out_vals->end());
    EXPECT_EQ(it_raw.second.in_val, it->second.in_val);
    EXPECT_EQ(it_raw.second.out_val, it->second.out_val);
  }
}

TEST(test_graph, test_get_stakes) {
  auto stakes_raw = neb::graph_algo::get_stakes(tgp_raw->internal_graph());
  auto stakes = neb::graph_algo::get_stakes(tgp->internal_graph());

  EXPECT_EQ(stakes_raw->size(), stakes->size());
  for (auto &it_raw : *stakes_raw) {
    auto it = stakes->find(it_raw.first);
    EXPECT_TRUE(it != stakes->end());
    EXPECT_EQ(it_raw.second, it->second);
  }
}

TEST(test_graph, test_get_in_out_degrees) {
  auto in_out_degrees_raw =
      neb::graph_algo::get_in_out_degrees(tgp_raw->internal_graph());
  auto in_out_degrees =
      neb::graph_algo::get_in_out_degrees(tgp->internal_graph());

  EXPECT_EQ(in_out_degrees_raw->size(), in_out_degrees->size());
  for (auto &it_raw : *in_out_degrees_raw) {
    auto it = in_out_degrees->find(it_raw.first);
    EXPECT_TRUE(it != in_out_degrees->end());
    EXPECT_EQ(it_raw.second.in_degree, it->second.in_degree);
    EXPECT_EQ(it_raw.second.out_degree, it->second.out_degree);
  }
}

TEST(test_graph, test_get_degree_sum) {
  auto degree_sum_raw =
      neb::graph_algo::get_degree_sum(tgp_raw->internal_graph());
  auto degree_sum = neb::graph_algo::get_degree_sum(tgp->internal_graph());

  EXPECT_EQ(degree_sum_raw->size(), degree_sum->size());
  for (auto &it_raw : *degree_sum_raw) {
    auto it = degree_sum->find(it_raw.first);
    EXPECT_TRUE(it != degree_sum->end());
    EXPECT_EQ(it_raw.second, it->second);
  }
}
