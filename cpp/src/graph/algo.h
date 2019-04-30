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
  static void non_recursive_remove_cycles_based_on_time_sequence(
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

  static bool decrease_graph_edges(
      const transaction_graph::internal_graph_t &graph,
      std::unordered_set<transaction_graph::vertex_descriptor_t> &dead_v,
      std::unordered_map<transaction_graph::vertex_descriptor_t, size_t>
          &dead_to,
      std::unordered_map<transaction_graph::vertex_descriptor_t, size_t>
          &to_dead);

private:
  static void bfs_decrease_graph_edges(
      const transaction_graph::internal_graph_t &graph,
      const std::unordered_set<transaction_graph::vertex_descriptor_t> &dead_v,
      std::unordered_set<transaction_graph::vertex_descriptor_t> &tmp_dead,
      std::unordered_map<transaction_graph::vertex_descriptor_t, size_t>
          &dead_to,
      std::unordered_map<transaction_graph::vertex_descriptor_t, size_t>
          &to_dead);

  static transaction_graph_ptr_t
  merge_two_graphs(transaction_graph_ptr_t tg,
                   const transaction_graph_ptr_t sg);
};
} // namespace neb
