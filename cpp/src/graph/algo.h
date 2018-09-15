#pragma once

#include "graph/common.h"
#include "graph/graph.h"

namespace neb {

void merge_edges_with_same_from_and_same_to(
    transaction_graph::internal_graph_t &graph);

auto find_a_cycle_based_on_time_sequence(
    const transaction_graph::internal_graph_t &graph) ->
    std::vector<transaction_graph::edge_descriptor_t>;

void remove_cycles_based_on_time_sequence(
    transaction_graph::internal_graph_t &graph);

transaction_graph_ptr merge_two_graphs(transaction_graph_ptr tg,
                                       const transaction_graph_ptr sg);

transaction_graph_ptr
merge_graphs(const std::vector<transaction_graph_ptr> &graphs);

void merge_topk_edges_with_same_from_and_same_to(
    transaction_graph::internal_graph_t &graph, uint32_t k = 3);

struct in_out_val {
  double in_val;
  double out_val;
};

auto get_in_out_vals(const transaction_graph::internal_graph_t &graph) ->
    std::unordered_map<std::string, in_out_val>;

auto get_stakes(const transaction_graph::internal_graph_t &graph) ->
    std::unordered_map<std::string, double>;

struct in_out_degree {
  int in_degree;
  int out_degree;
};

auto get_in_out_degrees(const transaction_graph::internal_graph_t &graph) ->
    std::unordered_map<std::string, in_out_degree>;

auto get_degree_sum(const transaction_graph::internal_graph_t &graph) ->
    std::unordered_map<std::string, int>;

}
