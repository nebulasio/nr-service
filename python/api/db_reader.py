#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/08/24
'''

import sys
sys.path.append('../')
import json

from graph.arango_graph import database


def read_from_transaction_with_duration(start_block, end_block):
    '''
    @start_block - block height start
    @end_block - block height end
    @return - time spend
    '''

    assert isinstance(start_block, int)
    assert isinstance(end_block, int)

    aql = '''for tx in transaction filter tx.height>=%d and tx.height<=%d
            return tx''' % (start_block, end_block)

    cursor = database.aql.execute(aql, count=True, batch_size=0x7fffffff)
    result = [doc for doc in cursor]
    return json.dumps(result)


def main():
    start_block = int(sys.argv[1])
    end_block = int(sys.argv[2])

    l = read_from_transaction_with_duration(start_block, end_block)
    print l


if __name__ == '__main__':
    main()
