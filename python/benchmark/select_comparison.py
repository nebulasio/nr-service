#!/usr/local/bin/python2.7

'''
Author: Chenmin Wang
Date: 2018/08/13
'''

import random
import time

import MySQLdb
from pyArango.connection import Connection
from ..graph.arango_graph import dbuser, dbpass, dbname
from ..graph.arango_graph import db


def select_mysql(start_block, end_block, block_interval, compared_times):
    '''
    @start_block - block height start
    @end_block - block height end
    @block_interval - block query range
    @compared_times - query times
    @return - None
    '''

    assert isinstance(start_block, int)
    assert isinstance(end_block, int)
    assert isinstance(block_interval, int)
    assert isinstance(compared_times, int)

    mysql_db = MySQLdb.connect(
        'localhost', dbuser, dbpass, dbname, charset='utf8')
    cursor = mysql_db.cursor()

    time_spend = float()
    for i in range(0, compared_times):
        s = start_block + random.random() * (
            end_block - start_block - block_interval)
        e = s + block_interval
        sql = '''select _from, _to, value, height, _timestamp, type_from,
                type_to, gas_used, gas_price, contract_address from
                nebulas_transaction_db where status=1 and height>%d and height<%d''' % (
                    s, e)
        # print i, sql

        try:
            time_begin = time.time()
            cursor.execute(sql)
            time_spend = time_spend + time.time() - time_begin
            results = cursor.fetchall()
            # print len(results)
        except Exception, e:
            print e

    print 'mysql select %d times with block interval %d, time spend: %fs' % (
        compared_times, block_interval, time_spend)


def select_arango(start_block, end_block, block_interval, compared_times):
    '''
    @start_block - block height start
    @end_block - block height end
    @block_interval - block query range
    @compared_times - query times
    @return - None
    '''

    assert isinstance(start_block, int)
    assert isinstance(end_block, int)
    assert isinstance(block_interval, int)
    assert isinstance(compared_times, int)

    time_spend = float()
    for i in range(0, compared_times):
        s = start_block + random.random() * (
            end_block - start_block - block_interval)
        e = s + block_interval
        aql = '''for tx in transaction filter tx.height>%d and tx.height<%d collect
                from=tx.from, to=tx.to, value=tx.value, height=tx.height, timestamp=tx.timestamp,
                type_from=tx.type_from, type_to=tx.type_to, gas_used=tx.gas_used,
                gas_price=tx.gas_price, contract_address=tx.contract_address return
                {from, to, value, height, timestamp, type_from, type_to, gas_used,
                gas_price, contract_address}''' % (s, e)
        # print i, aql

        time_begin = time.time()
        cursor = db.aql.execute(aql, count=True, batch_size=0x7fffffff)
        time_spend = time_spend + time.time() - time_begin
        # print cursor.count()

    print 'arango select %d times with block interval %d, time spend: %fs' % (
        compared_times, block_interval, time_spend)


def select_pyarango(start_block, end_block, block_interval, compared_times):
    '''
    @start_block - block height start
    @end_block - block height end
    @block_interval - block query range
    @compared_times - query times
    @return - None
    '''

    assert isinstance(start_block, int)
    assert isinstance(end_block, int)
    assert isinstance(block_interval, int)
    assert isinstance(compared_times, int)

    conn = Connection(username=dbuser, password=dbpass)
    pyarango_db = conn[dbname]

    time_spend = float()
    for i in range(0, compared_times):
        s = start_block + random.random() * (
            end_block - start_block - block_interval)
        e = s + block_interval
        aql = '''for tx in transaction filter tx.height>%d and tx.height<%d collect
                from=tx.from, to=tx.to, value=tx.value, height=tx.height, timestamp=tx.timestamp,
                type_from=tx.type_from, type_to=tx.type_to, gas_used=tx.gas_used,
                gas_price=tx.gas_price, contract_address=tx.contract_address return
                {from, to, value, height, timestamp, type_from, type_to, gas_used,
                gas_price, contract_address}''' % (s, e)
        # print i, aql

        time_begin = time.time()
        results = pyarango_db.AQLQuery(aql, batchSize=0x7fffffff)
        time_spend = time_spend + time.time() - time_begin
        # print len(results)

    print 'pyarango select %d times with block interval %d, time spend: %fs' % (
        compared_times, block_interval, time_spend)
