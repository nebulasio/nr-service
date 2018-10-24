#include "graph/algo.h"

namespace neb {

void graph_algo::dfs_find_a_cycle_from_vertex_based_on_time_sequence(
    const transaction_graph::vertex_descriptor_t &start_vertex,
    const transaction_graph::vertex_descriptor_t &v,
    const transaction_graph::internal_graph_t &graph,
    std::set<transaction_graph::vertex_descriptor_t> &visited,
    std::vector<transaction_graph::edge_descriptor_t> &edges, bool &has_cycle,
    std::vector<transaction_graph::edge_descriptor_t> &ret) {

  if (has_cycle) return;

  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::out_edge_iterator oeiterator_t;

  oeiterator_t oei, oei_end;

  // std::string s_addr = boost::get(boost::vertex_name_t(), graph, v);
  // std::cout << "cur vertex: " << s_addr << ", out edges: ";
  // for (boost::tie(oei, oei_end) = boost::out_edges(v, graph); oei != oei_end;
  // oei++) {
  // auto target = boost::target(*oei, graph);
  // std::string t_addr = boost::get(boost::vertex_name_t(), graph, target);
  // std::cout << t_addr << ", ";
  //}
  // std::cout << std::endl;

  for (boost::tie(oei, oei_end) = boost::out_edges(v, graph);
       oei != oei_end; oei++) {
    auto target = boost::target(*oei, graph);

    // std::string s_addr = boost::get(boost::vertex_name_t(), graph, v);
    // std::string t_addr = boost::get(boost::vertex_name_t(), graph, target);
    // std::cout << "s_addr: " << s_addr << ", t_addr: " << t_addr << std::endl;

    if (target == start_vertex) {
      if (!edges.empty()) {
        double ts = boost::get(boost::edge_timestamp_t(), graph, edges.back());
        double ts_next = boost::get(boost::edge_timestamp_t(), graph, *oei);
        if (ts >= ts_next) {
          continue;
        }
      }

      // has_cycle = true;
      edges.push_back(*oei);

      if (!ret.empty()) {

        double min_w_ret = -1;
        for (auto it = ret.begin(); it != ret.end(); it++) {
          double w_ret = boost::get(boost::edge_weight_t(), graph, *it);
          min_w_ret = (min_w_ret==-1 ? w_ret : fmin(min_w_ret, w_ret));
        }

        double min_w_cur = -1;
        for (auto it = edges.begin(); it != edges.end(); it++) {
          double w_cur = boost::get(boost::edge_weight_t(), graph, *it);
          min_w_cur = (min_w_cur==-1 ? w_cur : fmin(min_w_cur, w_cur));
        }

        if (min_w_cur >= min_w_ret) {
          edges.pop_back();
          continue;
        }
      }

      ret.clear();
      // std::cout << "cycle size: " << edges.size() << std::endl;
      for (auto it = edges.begin(); it != edges.end(); it++) {
        ret.push_back(*it);
      }
      // auto it = edges.begin();
      // auto source = boost::source(*it, graph);
      // std::string addr = boost::get(boost::vertex_name_t(), graph, source);
      // std::cout << "cycle: " << addr;
      // for (; it != edges.end(); it++) {
      // auto target = boost::target(*it, graph);
      // addr = boost::get(boost::vertex_name_t(), graph, target);
      // std::cout << "->" << addr;
      //}
      // std::cout << std::endl;
      edges.pop_back();
      continue;
    }
    if (visited.find(target) != visited.end()) {
      continue;
    }

    visited.insert(target);

    if (edges.empty()) {
      edges.push_back(*oei);
      dfs_find_a_cycle_from_vertex_based_on_time_sequence(
          start_vertex, target, graph, visited, edges, has_cycle, ret);
      edges.pop_back();
    } else {
      double ts = boost::get(boost::edge_timestamp_t(), graph, edges.back());
      double ts_next = boost::get(boost::edge_timestamp_t(), graph, *oei);
      if (ts < ts_next) {
        edges.push_back(*oei);
        dfs_find_a_cycle_from_vertex_based_on_time_sequence(
            start_vertex, target, graph, visited, edges, has_cycle, ret);
        edges.pop_back();
      }
    }

    // std::cout << "backtrace!" << std::endl;
    visited.erase(visited.find(target));
  }
  return;
}

std::vector<transaction_graph::edge_descriptor_t>
graph_algo::find_a_cycle_from_vertex_based_on_time_sequence(
    const transaction_graph::vertex_descriptor_t &v,
    const transaction_graph::internal_graph_t &graph) {

  std::vector<transaction_graph::edge_descriptor_t> ret;
  std::vector<transaction_graph::edge_descriptor_t> edges;
  std::set<transaction_graph::vertex_descriptor_t> visited;
  bool has_cycle = false;

  visited.insert(v);
  dfs_find_a_cycle_from_vertex_based_on_time_sequence(v, v, graph,
                                                      visited, edges, has_cycle, ret);
  return ret;
}

std::vector<transaction_graph::edge_descriptor_t>
graph_algo::find_a_cycle_based_on_time_sequence(
    const transaction_graph::internal_graph_t &graph) {
  std::vector<transaction_graph::vertex_descriptor_t> to_visit;

  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::vertex_iterator viterator_t;

  viterator_t vi, vi_end;

  for (boost::tie(vi, vi_end) = boost::vertices(graph); vi != vi_end; vi++) {
    to_visit.push_back(*vi);
  }

  for (auto it = to_visit.begin(); it != to_visit.end(); it++) {
    auto ret = find_a_cycle_from_vertex_based_on_time_sequence(*it, graph);
    if (!ret.empty()) {
      return ret;
    }
  }
  return std::vector<transaction_graph::edge_descriptor_t>();
}

void graph_algo::remove_cycles_based_on_time_sequence(
    transaction_graph::internal_graph_t &graph) {

  std::vector<transaction_graph::edge_descriptor_t> ret =
      find_a_cycle_based_on_time_sequence(graph);
  while (!ret.empty()) {

    double min_w = -1;
    for (auto it = ret.begin(); it != ret.end(); it++) {
      double w = boost::get(boost::edge_weight_t(), graph, *it);
      min_w = (min_w == -1 ? w : fmin(min_w, w));
    }

    for (auto it = ret.begin(); it != ret.end(); it++) {
      double w = boost::get(boost::edge_weight_t(), graph, *it);
      boost::put(boost::edge_weight_t(), graph, *it, w - min_w);
      if (w == min_w) {
        boost::remove_edge(*it, graph);
      }
    }

    // LOG(INFO) << "cycle min_w: " << min_w;
    ret = find_a_cycle_based_on_time_sequence(graph);
  }
}

void graph_algo::merge_edges_with_same_from_and_same_to(
    transaction_graph::internal_graph_t &graph) {

  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::vertex_iterator viterator_t;
  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::vertex_descriptor vdescriptor_t;
  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::out_edge_iterator oeiterator_t;

  viterator_t vi, vi_end;

  for (boost::tie(vi, vi_end) = boost::vertices(graph); vi != vi_end; vi++) {
    oeiterator_t oei, oei_end;
    std::unordered_map<vdescriptor_t, double> target_and_vals;
    for (boost::tie(oei, oei_end) = boost::out_edges(*vi, graph);
         oei != oei_end; oei++) {
      auto target = boost::target(*oei, graph);
      double val = boost::get(boost::edge_weight_t(), graph, *oei);
      if (target_and_vals.find(target) == target_and_vals.end()) {
        target_and_vals.insert(std::make_pair(target, val));
      } else {
        target_and_vals[target] += val;
      }
    }

    bool removed_all_edges = false;
    while (!removed_all_edges) {
      removed_all_edges = true;
      for (boost::tie(oei, oei_end) = boost::out_edges(*vi, graph);
           oei != oei_end; oei++) {
        removed_all_edges = false;
        boost::remove_edge(oei, graph);
        break;
      }
    }

    for (auto it = target_and_vals.begin(); it != target_and_vals.end(); it++) {
      boost::add_edge(*vi, it->first, {it->second, 0}, graph);
    }
  }
  return;
}

transaction_graph_ptr
graph_algo::merge_two_graphs(transaction_graph_ptr tg,
                             const transaction_graph_ptr sg) {

  transaction_graph::internal_graph_t sgi = sg->internal_graph();

  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::vertex_iterator viterator_t;
  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::out_edge_iterator oeiterator_t;

  viterator_t vi, vi_end;

  for (boost::tie(vi, vi_end) = boost::vertices(sgi); vi != vi_end; vi++) {
    oeiterator_t oei, oei_end;
    for (boost::tie(oei, oei_end) = boost::out_edges(*vi, sgi); oei != oei_end;
         oei++) {
      auto source = boost::source(*oei, sgi);
      std::string from = boost::get(boost::vertex_name_t(), sgi, source);
      auto target = boost::target(*oei, sgi);
      std::string to = boost::get(boost::vertex_name_t(), sgi, target);
      double w = boost::get(boost::edge_weight_t(), sgi, *oei);

      tg->add_edge(from, to, w, 0);
    }
  }
  return tg;
}

transaction_graph_ptr
graph_algo::merge_graphs(const std::vector<transaction_graph_ptr> &graphs) {
  if (!graphs.empty()) {
    transaction_graph_ptr ret = *graphs.begin();
    for (auto it = graphs.begin() + 1; it != graphs.end(); it++) {
      transaction_graph_ptr ptr = *it;
      ret = merge_two_graphs(ret, ptr);
    }
    return ret;
  }
  return nullptr;
}

void graph_algo::merge_topk_edges_with_same_from_and_same_to(
    transaction_graph::internal_graph_t &graph, uint32_t k) {

  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::vertex_iterator viterator_t;
  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::vertex_descriptor vdescriptor_t;
  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::out_edge_iterator oeiterator_t;

  viterator_t vi, vi_end;

  struct edge_st {
    double weight;
    transaction_graph::edge_descriptor_t edescriptor;
  };

  auto cmp = [] (const edge_st &e1, const edge_st &e2) -> bool {
    return e1.weight > e2.weight;
  };

  typedef std::priority_queue<edge_st, std::vector<edge_st>, decltype(cmp)> pq_t;

  for (boost::tie(vi, vi_end) = boost::vertices(graph); vi != vi_end; vi++) {
    std::unordered_map<vdescriptor_t, double> target_and_vals;
    std::unordered_map<vdescriptor_t, pq_t> target_and_minheap;

    oeiterator_t oei, oei_end;
    for (boost::tie(oei, oei_end) = boost::out_edges(*vi, graph);
         oei != oei_end; oei++) {
      auto target = boost::target(*oei, graph);
      double val = boost::get(boost::edge_weight_t(), graph, *oei);
      if (target_and_vals.find(target) == target_and_vals.end()) {
        target_and_vals.insert(std::make_pair(target, val));
        pq_t min_heap(cmp);
        min_heap.push(edge_st{val, *oei});
        target_and_minheap.insert(std::make_pair(target, min_heap));
      } else {
        pq_t &min_heap = target_and_minheap.find(target)->second;
        if (min_heap.size() < k) {
          min_heap.push(edge_st{val, *oei});
          target_and_vals[target] += val;
        } else {
          edge_st e = min_heap.top();
          if (val > e.weight) {
            // boost::remove_edge(e.edescriptor, graph);
            min_heap.pop();
            target_and_vals[target] -= e.weight;
            min_heap.push(edge_st{val, *oei});
            target_and_vals[target] += val;
          } else {
            // boost::remove_edge(oei, graph);
          }
        }
      }
    }

    bool removed_all_edges = false;
    while (!removed_all_edges) {
      removed_all_edges = true;
      for (boost::tie(oei, oei_end) = boost::out_edges(*vi, graph);
           oei != oei_end; oei++) {
        removed_all_edges = false;
        boost::remove_edge(oei, graph);
        break;
      }
    }

    for (auto it = target_and_vals.begin(); it != target_and_vals.end(); it++) {
      boost::add_edge(*vi, it->first, {it->second, 0}, graph);
    }
  }
}

std::shared_ptr<std::unordered_map<std::string, in_out_val>>
graph_algo::get_in_out_vals(const transaction_graph::internal_graph_t &graph) {

  std::unordered_map<std::string, in_out_val> ret;

  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::vertex_iterator viterator_t;
  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::in_edge_iterator ieiterator_t;
  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::out_edge_iterator oeiterator_t;

  viterator_t vi, vi_end;

  for (boost::tie(vi, vi_end) = boost::vertices(graph); vi != vi_end; vi++) {
    ieiterator_t iei, iei_end;
    double in_val = 0;
    for (boost::tie(iei, iei_end) = boost::in_edges(*vi, graph); iei != iei_end;
         iei++) {
      double val = boost::get(boost::edge_weight_t(), graph, *iei);
      in_val += val;
    }

    oeiterator_t oei, oei_end;
    double out_val = 0;
    for (boost::tie(oei, oei_end) = boost::out_edges(*vi, graph);
         oei != oei_end; oei++) {
      double val = boost::get(boost::edge_weight_t(), graph, *oei);
      out_val += val;
    }

    std::string addr = boost::get(boost::vertex_name_t(), graph, *vi);
    ret.insert(std::make_pair(addr, in_out_val{in_val, out_val}));
  }
  return std::make_shared<std::unordered_map<std::string, in_out_val>>(ret);
}

std::shared_ptr<std::unordered_map<std::string, double>>
graph_algo::get_stakes(const transaction_graph::internal_graph_t &graph) {

  std::unordered_map<std::string, double> ret;

  auto it_in_out_vals = get_in_out_vals(graph);
  auto in_out_vals = *it_in_out_vals;
  for (auto it = in_out_vals.begin(); it != in_out_vals.end(); it++) {
    ret.insert(
        std::make_pair(it->first, it->second.in_val - it->second.out_val));
  }
  return std::make_shared<std::unordered_map<std::string, double>>(ret);
}

std::shared_ptr<std::unordered_map<std::string, in_out_degree>>
graph_algo::get_in_out_degrees(
    const transaction_graph::internal_graph_t &graph) {

  std::unordered_map<std::string, in_out_degree> ret;

  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::vertex_iterator viterator_t;
  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::in_edge_iterator ieiterator_t;
  typedef typename boost::graph_traits<
      transaction_graph::internal_graph_t>::out_edge_iterator oeiterator_t;

  viterator_t vi, vi_end;

  for (boost::tie(vi, vi_end) = boost::vertices(graph); vi != vi_end; vi++) {
    ieiterator_t iei, iei_end;
    int in_degree = 0;
    for (boost::tie(iei, iei_end) = boost::in_edges(*vi, graph); iei != iei_end;
         iei++) {
      in_degree++;
    }

    oeiterator_t oei, oei_end;
    int out_degree = 0;
    for (boost::tie(oei, oei_end) = boost::out_edges(*vi, graph);
         oei != oei_end; oei++) {
      out_degree++;
    }

    std::string addr = boost::get(boost::vertex_name_t(), graph, *vi);
    ret.insert(std::make_pair(addr, in_out_degree{in_degree, out_degree}));
  }
  return std::make_shared<std::unordered_map<std::string, in_out_degree>>(ret);
}

std::shared_ptr<std::unordered_map<std::string, int>>
graph_algo::get_degree_sum(const transaction_graph::internal_graph_t &graph) {

  std::unordered_map<std::string, int> ret;

  auto it_in_out_degrees = get_in_out_degrees(graph);
  auto in_out_degrees = *it_in_out_degrees;
  for (auto it = in_out_degrees.begin(); it != in_out_degrees.end(); it++) {
    ret.insert(std::make_pair(it->first,
                              it->second.in_degree + it->second.out_degree));
  }
  return std::make_shared<std::unordered_map<std::string, int>>(ret);
}

} // namespace neb
