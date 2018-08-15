#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/08/13
'''

import random
import time

import MySQLdb
import pyArango
from pyArango.connection import Connection
from ..graph.arango_graph import dbuser, dbpass, dbname
from ..graph.arango_graph import database


def select_mysql(mysql_db, start_block, end_block):
    '''
    @mysql_db - mysql database connection instance
    @start_block - block height start
    @end_block - block height end
    @return - time spend
    '''

    assert isinstance(mysql_db, MySQLdb.connections.Connection)
    assert isinstance(start_block, int)
    assert isinstance(end_block, int)

    cursor = mysql_db.cursor()

    time_spend = float()
    sql = '''select _from, _to, value, height, _timestamp, type_from,
            type_to, gas_used, gas_price, contract_address from
            nebulas_transaction_db where status=1 and height>%d and height<%d''' % (
                start_block, end_block)
    # print i, sql

    try:
        time_begin = time.time()
        cursor.execute(sql)
        time_spend = time_spend + time.time() - time_begin
        results = cursor.fetchall()
        # print len(results)
    except Exception, e:
        print e

    return time_spend


def select_arango(arangodb, start_block, end_block):
    '''
    @arangodb - arango client database instance
    @start_block - block height start
    @end_block - block height end
    @return - time spend
    '''

    assert isinstance(start_block, int)
    assert isinstance(end_block, int)

    time_spend = float()
    aql = '''for tx in transaction filter tx.status==1 and tx.height>%d and tx.height<%d collect
            from=tx.from, to=tx.to, value=tx.value, height=tx.height, timestamp=tx.timestamp,
            type_from=tx.type_from, type_to=tx.type_to, gas_used=tx.gas_used,
            gas_price=tx.gas_price, contract_address=tx.contract_address return
            {from, to, value, height, timestamp, type_from, type_to, gas_used,
            gas_price, contract_address}''' % (start_block, end_block)
    # print i, aql

    time_begin = time.time()
    cursor = arangodb.aql.execute(aql, count=True, batch_size=0x7fffffff)
    time_spend = time_spend + time.time() - time_begin
    # print cursor.count()

    return time_spend


def select_pyarango(pyarango_db, start_block, end_block):
    '''
    @pyarango_db - pyarango database handler
    @start_block - block height start
    @end_block - block height end
    @return - time spend
    '''

    assert isinstance(pyarango_db, pyArango.database.DBHandle)
    assert isinstance(start_block, int)
    assert isinstance(end_block, int)

    time_spend = float()
    aql = '''for tx in transaction filter tx.status==1 and tx.height>%d and tx.height<%d collect
            from=tx.from, to=tx.to, value=tx.value, height=tx.height, timestamp=tx.timestamp,
            type_from=tx.type_from, type_to=tx.type_to, gas_used=tx.gas_used,
            gas_price=tx.gas_price, contract_address=tx.contract_address return
            {from, to, value, height, timestamp, type_from, type_to, gas_used,
            gas_price, contract_address}''' % (start_block, end_block)
    # print i, aql

    time_begin = time.time()
    results = pyarango_db.AQLQuery(aql, batchSize=0x7fffffff)
    time_spend = time_spend + time.time() - time_begin
    # print len(results)

    return time_spend


def benchmark(start_block, end_block, block_interval, compared_times):
    '''
    usage - benchmark for mysqldb and arangodb comparison
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

    conn = Connection(username=dbuser, password=dbpass)
    pyarango_db = conn[dbname]

    while compared_times > 0:
        s = start_block + random.random() * (
            end_block - start_block - block_interval)
        e = s + block_interval

        s = int(s)
        e = int(e)

        print select_mysql(mysql_db, s,
                           e), select_arango(database, s, e), select_pyarango(
                               pyarango_db, s, e)
        compared_times = compared_times - 1
