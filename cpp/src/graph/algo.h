#pragma once

#include "graph/common.h"
#include "graph/graph.h"

namespace neb {

struct in_out_val {
  double in_val;
  double out_val;
};

struct in_out_degree {
  int in_degree;
  int out_degree;
};

class graph_algo {
public:
  static void remove_cycles_based_on_time_sequence(
      transaction_graph::internal_graph_t &graph);

  static void merge_edges_with_same_from_and_same_to(
      transaction_graph::internal_graph_t &graph);

  static transaction_graph_ptr_t
  merge_graphs(const std::vector<transaction_graph_ptr_t> &graphs);

  static void merge_topk_edges_with_same_from_and_same_to(
      transaction_graph::internal_graph_t &graph, uint32_t k = 3);

  static auto get_in_out_vals(const transaction_graph::internal_graph_t &graph)
      -> std::shared_ptr<std::unordered_map<std::string, in_out_val>>;

  static auto get_stakes(const transaction_graph::internal_graph_t &graph)
      -> std::shared_ptr<std::unordered_map<std::string, double>>;

  static auto
  get_in_out_degrees(const transaction_graph::internal_graph_t &graph)
      -> std::shared_ptr<std::unordered_map<std::string, in_out_degree>>;

  static auto get_degree_sum(const transaction_graph::internal_graph_t &graph)
      -> std::shared_ptr<std::unordered_map<std::string, int>>;

private:
  static void dfs_find_a_cycle_from_vertex_based_on_time_sequence(
      const transaction_graph::vertex_descriptor_t &start_vertex,
      const transaction_graph::vertex_descriptor_t &v,
      const transaction_graph::internal_graph_t &graph,
      std::set<transaction_graph::vertex_descriptor_t> &visited,
      std::vector<transaction_graph::edge_descriptor_t> &edges, bool &has_cycle,
      std::vector<transaction_graph::edge_descriptor_t> &ret);

  static auto find_a_cycle_from_vertex_based_on_time_sequence(
      const transaction_graph::vertex_descriptor_t &v,
      const transaction_graph::internal_graph_t &graph)
      -> std::vector<transaction_graph::edge_descriptor_t>;

  static auto find_a_cycle_based_on_time_sequence(
      const transaction_graph::internal_graph_t &graph)
      -> std::vector<transaction_graph::edge_descriptor_t>;

  static transaction_graph_ptr_t
  merge_two_graphs(transaction_graph_ptr_t tg,
                   const transaction_graph_ptr_t sg);
};
} // namespace neb
