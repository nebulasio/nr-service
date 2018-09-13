#include "blockchain.h"
#include "graph.h"

typedef typename boost::graph_traits<
    neb::transaction_graph::internal_graph_t>::vertex_iterator viterator_t;
typedef typename boost::graph_traits<
    neb::transaction_graph::internal_graph_t>::out_edge_iterator oeiterator_t;

typedef std::shared_ptr<neb::transaction_db_interface> transaction_ptr_t;
typedef neb::transaction_db<neb::nebulas_db> transaction_db_t;
typedef neb::transaction_db<neb::eth_db> eth_transaction_db_t;

typedef std::shared_ptr<neb::account_db_interface> account_ptr_t;
typedef neb::nebulas::nebulas_account_db nebulas_account_db_t;

extern transaction_db_t nt_db;
extern eth_transaction_db_t et_db;
extern transaction_ptr_t transaction_db_ptr;

extern nebulas_account_db_t na_db;
extern account_ptr_t account_db_ptr;

