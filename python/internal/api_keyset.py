#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2019/01/10
'''

import sys
sys.path.append('../')
import json

from db_conf import db_client


def get_keys(dbname, collection, batch_size, field):
    '''
    @dbname - nebulas or ethereum database name
    @collection - collection name
    @batch_size - batch of result size
    @field - specify db collection field
    @return - current database collection keys
    '''

    assert isinstance(dbname, str)
    assert isinstance(collection, str)
    assert isinstance(batch_size, int)
    assert isinstance(field, str)

    limit = 1 << 8
    aql = '''for item in %s collect field=item.%s limit %s return field''' % (collection, field, limit)

    cursor = db_client[dbname].aql.execute(aql, batch_size=batch_size, ttl=60)
    d = {
        'result': list(cursor.batch()),
        'has_more': cursor.has_more(),
        'id': cursor.id
    }
    return json.dumps(d)
