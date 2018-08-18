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
} // namespace neb
