import sys

import arango_graph
from arango_graph import db, logger
import select_comparison

# arango_graph.clear_db(db)

# txs = arango_graph.get_transactions_from_mysql(sys.argv[1], sys.argv[2])
# logger.info('get transaction from mysql')

# arango_graph.build_arango_graph(db, txs)
# logger.info('build arango graph')

start_block = int(sys.argv[1])
end_block = int(sys.argv[2])
block_interval = [240, 1440, 2880, 5760, 17280, 40320]
compared_times = int(sys.argv[4])

for b in block_interval:
    select_comparison.select_mysql(start_block, end_block, interval, compared_times)
    select_comparison.select_arango(start_block, end_block, interval, compared_times)
    select_comparison.select_pyarango(start_block, end_block, interval, compared_times)
