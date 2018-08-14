#!/usr/local/bin/python2.7

'''
Author: Chenmin Wang
Date: 2018/08/13
'''

import sys
from .util.util import LOG

from .graph import arango_graph
from .graph.arango_graph import db

from .benchmark import select_comparison


def main():
    '''
    func main
    '''

    # arango_graph.clear_db(db)
    # arango_graph.drop_db(db)
    # arango_graph.create_db(db)

    start_block = int(sys.argv[1])
    end_block = int(sys.argv[2])
    block_interval = [240, 1440, 2880, 5760, 17280, 40320]
    compared_times = int(sys.argv[3])

    txs = arango_graph.get_transactions_from_mysql(sys.argv[1], sys.argv[2])
    LOG.info('get transaction from mysql')

    arango_graph.build_arango_graph(db, txs)
    LOG.info('build arango graph')

    for b in block_interval:
        select_comparison.select_mysql(start_block, end_block, b, compared_times)
        select_comparison.select_arango(start_block, end_block, b, compared_times)
        select_comparison.select_pyarango(start_block, end_block, b, compared_times)


if __name__ == '__main__':
    main()
