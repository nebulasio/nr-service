#!/usr/local/bin/python2.7

'''
Author: Chenmin Wang
Date: 2018/08/13
'''

import sys
from .util.util import LOG

from .graph import arango_graph
from .graph.arango_graph import database

from .benchmark import select_comparison


def main():
    '''
    usage - func main
    '''

    # arango_graph.clear_db(database)
    # arango_graph.drop_db(database)
    # arango_graph.create_db(database)

    start_block = int(sys.argv[1])
    end_block = int(sys.argv[2])
    block_interval = [240, 1440, 2880, 5760, 17280, 40320]
    compared_times = int(sys.argv[3])

    # txs = arango_graph.get_transactions_from_mysql(start_block, end_block)
    # LOG.info('get transaction from mysql')

    # arango_graph.build_arango_graph(database, txs)
    # LOG.info('build arango graph')

    select_comparison.benchmark(start_block, end_block, 70, compared_times)

if __name__ == '__main__':
    main()
