#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/11/13
'''

import sys
sys.path.append('../')
import json

from db_conf import db_client


def get_transactions(dbname, batch_size, address, start_ts, end_ts, limit=1<<15):
    '''
    @dbname - nebulas or ethereum database name
    @batch_size - batch of result size
    @address - normal address or contract address
    @start_ts - start timestamp
    @end_ts - end timestamp
    @return - current batch of transaction results
    '''

    assert isinstance(dbname, str)
    assert isinstance(batch_size, int)
    assert isinstance(address, str)
    assert isinstance(start_ts, str)
    assert isinstance(end_ts, str)

    aql = '''for tx in transaction'''

    if address:
        aql = '''%s filter tx.from=='%s' or tx.to=='%s' ''' % (aql, address, address)

    if start_ts and end_ts:
        aql = '''%s filter tx.timestamp>='%s' and tx.timestamp<'%s' ''' % (aql, start_ts, end_ts)

    aql = '''%s limit %s return tx''' % (aql, limit)

    cursor = db_client[dbname].aql.execute(aql, batch_size=batch_size, ttl=60)
    d = {
        'result': list(cursor.batch()),
        'has_more': cursor.has_more(),
        'id': cursor.id
    }
    return json.dumps(d)


def main():
    ret = get_transactions('nebulas', 5, 'n1Wt2VbPAR6TttM17HQXscCyWBrFe36HeYC')
    print ret


if __name__ == '__main__':
    main()
