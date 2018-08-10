import random
import sys, time

import MySQLdb
from arango_graph_build import dbuser, dbpass, dbname
from arango_graph_build import db

lines = 100

start_block = 380000
end_block = 385000

hour_block_interval = 3600 / 15
half_day_block_interval = 12 * hour_block_interval
block_interval = half_day_block_interval


def select_mysql():
    db = MySQLdb.connect('localhost', dbuser, dbpass, dbname, charset='utf8')
    cursor = db.cursor()

    time_spend = float()
    for i in range(0, lines):
        s = start_block + random.random() * (
            end_block - start_block - block_interval)
        e = s + block_interval
        sql = 'select _from, _to, value, height, _timestamp, type_from, type_to, gas_used, gas_price, contract_address from nebulas_transaction_db where status=1 and height>%d and height<%d' % (
            s, e)
        # print i, sql
        try:
            time_begin = time.time()
            cursor.execute(sql)
            time_spend = time_spend + time.time() - time_begin
            results = cursor.fetchall()
            # print len(results)
        except:
            print 'exception'
    print 'mysql select %d times with block interval %d, time spend: %fs' % (
        lines, block_interval, time_spend)


# select_mysql()


def select_arango():
    time_spend = float()
    for i in range(0, lines):
        s = start_block + random.random() * (
            end_block - start_block - block_interval)
        e = s + block_interval
        aql = 'for tx in transaction filter tx.height>%d and tx.height<%d collect from=tx.from, to=tx.to, value=tx.value, height=tx.height, timestamp=tx.timestamp, type_from=tx.type_from, type_to=tx.type_to, gas_used=tx.gas_used, gas_price=tx.gas_price, contract_address=tx.contract_address return {from, to, value, height, timestamp, type_from, type_to, gas_used, gas_price, contract_address}' % (
            s, e)
        # print i, aql
        time_begin = time.time()
        cursor = db.aql.execute(aql, count=True, batch_size=0x7fffffff)
        time_spend = time_spend + time.time() - time_begin
        # print cursor.count()
    print 'arango select %d times with block interval %d, time spend: %fs' % (
        lines, block_interval, time_spend)


# select_arango()
