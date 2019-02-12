#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2019/01/16
'''

import sys
sys.path.append('../')
import json

from db_conf import db_client


def get_accounts(dbname, account_type, batch_size):
    '''
    @dbname - nebulas or ethereum database name
    @account_type - account type, normal or contract
    @batch_size - batch of result size
    @return - accounts information
    '''

    assert isinstance(dbname, str)
    assert isinstance(account_type, str)
    assert isinstance(batch_size, int)

    aql = '''for acc in account filter acc.type=='%s' return acc''' % account_type

    cursor = db_client[dbname].aql.execute(aql, batch_size=batch_size, ttl=60)
    d = {
        'result': list(cursor.batch()),
        'has_more': cursor.has_more(),
        'id': cursor.id
    }
    return json.dumps(d)

