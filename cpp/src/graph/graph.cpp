#include "graph/graph.h"
#include <boost/graph/graphviz.hpp>
#include <map>

namespace neb {

transaction_graph::transaction_graph() : m_cur_max_index(0) {}

void transaction_graph::add_edge(const std::string &from, const std::string &to,
                                 double val, double ts) {
  uint64_t from_vertex, to_vertex;

  auto tmp_func = [&](const std::string &addr) {
    uint64_t ret;
    if (m_addr_to_vertex.find(addr) != m_addr_to_vertex.end()) {
      ret = m_addr_to_vertex[addr];
    } else {
      ret = m_cur_max_index;
      m_cur_max_index++;
      m_vertex_to_addr.insert(std::make_pair(ret, addr));
      m_addr_to_vertex.insert(std::make_pair(addr, ret));
    }
    return ret;
  };

  from_vertex = tmp_func(from);
  to_vertex = tmp_func(to);

  boost::add_edge(from_vertex, to_vertex, {val, ts}, m_graph);

  boost::put(boost::vertex_name_t(), m_graph, from_vertex, from);
  boost::put(boost::vertex_name_t(), m_graph, to_vertex, to);
}

void transaction_graph::write_to_graphviz(const std::string &filename) {
  std::map<std::string, std::string> graph_attr, vertex_attr, edge_attr;
  // graph_attr["size"] = "3,3";
  // graph_attr["rankdir"] = "LR";
  // graph_attr["ratio"] = "fill";
  // vertex_attr["shape"] = "circle";

  std::ofstream of;
  of.open(filename);
  boost::dynamic_properties dp;
  dp.property("node_id", boost::get(boost::vertex_name, m_graph));

  boost::write_graphviz(
      of, m_graph,
      boost::make_label_writer(boost::get(boost::vertex_name, m_graph)),
      boost::make_label_writer(boost::get(boost::edge_weight, m_graph)),
      boost::make_graph_attributes_writer(graph_attr, vertex_attr, edge_attr));

  of.close();
}

bool transaction_graph::read_from_graphviz(const std::string &filename,
                                           internal_graph_t &graph) {
  std::ifstream ifs(filename);
  if (!ifs) {
    return false;
  }

  std::stringstream ss;
  ss << ifs.rdbuf();
  ifs.close();

  boost::dynamic_properties dp(boost::ignore_other_properties);
  dp.property("label", boost::get(boost::vertex_name, graph));
  dp.property("label", boost::get(boost::edge_weight, graph));
  return boost::read_graphviz(ss, graph, dp);
}

transaction_graph_ptr_t build_graph_from_internal(
    const transaction_graph::internal_graph_t &internal_graph) {

  transaction_graph_ptr_t tg_ptr = std::make_shared<transaction_graph>();
  auto sgi = internal_graph;

  transaction_graph::viterator_t vi, vi_end;
  for (boost::tie(vi, vi_end) = boost::vertices(sgi); vi != vi_end; vi++) {
    transaction_graph::oeiterator_t oei, oei_end;
    for (boost::tie(oei, oei_end) = boost::out_edges(*vi, sgi); oei != oei_end;
         oei++) {
      auto source = boost::source(*oei, sgi);
      std::string from = boost::get(boost::vertex_name_t(), sgi, source);
      auto target = boost::target(*oei, sgi);
      std::string to = boost::get(boost::vertex_name_t(), sgi, target);
      double w = boost::get(boost::edge_weight_t(), sgi, *oei);
      double t = boost::get(boost::edge_timestamp_t(), sgi, *oei);

      tg_ptr->add_edge(from, to, w, t);
    }
  }
  return tg_ptr;
}

} // namespace neb
