#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/11/13
'''

import sys
sys.path.append('../')
import json

from db_conf import db_client


def get_nr_by_date(dbname, batch_size, date, collection):
    '''
    @dbname - nebulas or ethereum database name
    @batch_size - batch of result size
    @date - year/month/day
    @collection - collection name, default 'nr'
    @return - current batch of nr results
    '''

    assert isinstance(dbname, str)
    assert isinstance(batch_size, int)
    assert isinstance(date, str)
    assert isinstance(collection, str)

    aql = '''for item in %s filter item.date=='%s' return item''' % (collection, date)

    cursor = db_client[dbname].aql.execute(aql, batch_size=batch_size, ttl=60)
    d = {
        'result': list(cursor.batch()),
        'has_more': cursor.has_more(),
        'id': cursor.id
    }
    return json.dumps(d)


def get_nr_by_address(dbname, batch_size, address, collection):
    '''
    @dbname - nebulas or ethereum database name
    @batch_size - batch of result size
    @address - normal address
    @collection - collection name, default 'nr'
    @return - current batch of nr results
    '''

    assert isinstance(dbname, str)
    assert isinstance(batch_size, int)
    assert isinstance(address, str)
    assert isinstance(collection, str)

    aql = '''for item in %s filter item.address=='%s' return item''' % (collection, address)

    cursor = db_client[dbname].aql.execute(aql, batch_size=batch_size, ttl=60)
    d = {
        'result': list(cursor.batch()),
        'has_more': cursor.has_more(),
        'id': cursor.id
    }
    return json.dumps(d)


def get_nr_by_date_address(dbname, batch_size, date, address, collection):
    '''
    @dbname - nebulas or ethereum database name
    @batch_size - batch of result size
    @date - year/month/day
    @address - normal address
    @collection - collection name, default 'nr'
    @return - current batch of nr results
    '''

    assert isinstance(dbname, str)
    assert isinstance(batch_size, int)
    assert isinstance(date, str)
    assert isinstance(address, str)
    assert isinstance(collection, str)

    aql = '''for item in %s filter item.date=='%s' and
                item.address=='%s' return item''' % (collection, date, address)

    cursor = db_client[dbname].aql.execute(aql, batch_size=batch_size, ttl=60)
    d = {
        'result': list(cursor.batch()),
        'has_more': cursor.has_more(),
        'id': cursor.id
    }
    return json.dumps(d)


def main():
    ret = get_nr_by_date('nebulas', 3, '20181111')
    print ret


if __name__ == '__main__':
    main()
