import sys

import arango_graph
from arango_graph import db, logger

arango_graph.clear_db(db)

txs = arango_graph.get_transactions_from_mysql(sys.argv[1], sys.argv[2])
logger.info('get transaction from mysql')

arango_graph.build_arango_graph(db, txs)
logger.info('build arango graph')
