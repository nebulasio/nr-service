#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/08/13
'''

import sys
sys.path.append('../')
import os
from util.util import LOG

import MySQLdb
import arango
from arango import ArangoClient

dbuser = os.environ['DB_USER_NAME']
dbpass = os.environ['DB_PASSWORD']
dbname = os.environ['NEBULAS_DB']
client = ArangoClient(protocol='http', host='localhost', port=8529)
database = client.db(username=dbuser, password=dbpass, name=dbname)

collection_list = ['height', 'address', 'transaction']
edge_list = ['height_next', 'height_txs', 'from_txs', 'txs_to']


def create_edge_definition(graph, edge_collection, from_vertex_collection,
                           to_vertex_collection):
    '''
    usage - create edge collection from a vertex to another on graph
    @graph - arango graph database instance
    @edge_collection - edge collection name
    @from_vertex_collection - from vertex collection name
    @to_vertex_collection - to verrex collection name
    @return - None
    '''

    assert isinstance(graph, arango.graph.Graph)
    assert isinstance(edge_collection, str)
    assert isinstance(from_vertex_collection, str)
    assert isinstance(to_vertex_collection, str)

    if not graph.has_edge_definition(edge_collection):
        graph.create_edge_definition(
            edge_collection=edge_collection,
            from_vertex_collections=[from_vertex_collection],
            to_vertex_collections=[to_vertex_collection])
    return


def create_db(db):
    '''
    useage - create database 'nebulas'
           - create vertex collection 'height', 'address', transaction'
           - create index on collection 'transaction'
           - create graph 'txs_graph'
           - create edge collection 'height_next', 'height_txs', 'from_txs', 'txs_to'
    @db - arango client database instance
    @return - None
    '''

    assert isinstance(db, arango.database.StandardDatabase)

    if not db.has_database(dbname):
        db.create_database(dbname)

    # collection account
    if not db.has_collection('account'):
        db.create_collection('account')
    if db.has_collection('account'):
        collection = db.collection('account')
        collection.add_hash_index(fields=['address'])
        collection.add_hash_index(fields=['type'])
        collection.add_skiplist_index(fields=['height'])
        collection.add_skiplist_index(fields=['create_at'])

    # transaction graph
    for collection in collection_list:
        if not db.has_collection(collection):
            db.create_collection(collection)

    if db.has_collection('transaction'):
        collection = db.collection('transaction')
        collection.add_skiplist_index(fields=['height'])
        collection.add_hash_index(fields=['from'])
        collection.add_hash_index(fields=['to'])
        collection.add_skiplist_index(fields=['timestamp'])
        collection.add_hash_index(fields=['status'])

    graph = db.create_graph('txs_graph') if not db.has_graph(
        'txs_graph') else db.graph('txs_graph')

    for collection in collection_list:
        if not graph.has_vertex_collection(collection):
            graph.create_vertex_collection(collection)

    create_edge_definition(graph, 'height_next', 'height', 'height')
    create_edge_definition(graph, 'height_txs', 'height', 'transaction')
    create_edge_definition(graph, 'from_txs', 'address', 'transaction')
    create_edge_definition(graph, 'txs_to', 'transaction', 'address')


def drop_db(db):
    '''
    usage - drop graph, vertex_collection, edge_collection
    @db - arango client database instance
    @return - None
    '''

    assert isinstance(db, arango.database.StandardDatabase)

    if db.has_graph('txs_graph'):
        db.delete_graph('txs_graph')

    for collection in db.collections():
        if collection['system'] is False:
            name = collection['name']
            db.delete_collection(name)
    return


def clear_db(db):
    '''
    usage - truncate vertex_collection, edge_collection
    @db - arango client database instance
    @return - None
    '''

    assert isinstance(db, arango.database.StandardDatabase)

    for collection in db.collections():
        if collection['system'] is False:
            name = collection['name']
            db.collection(name).truncate()
    return


def get_transactions_from_mysql(start_block, end_block):
    '''
    usage - get transactions from mysql with start and end interval
    @start_block - block height start
    @end_block - block height end
    @return - transactions tuple
    '''

    assert isinstance(start_block, int)
    assert isinstance(end_block, int)

    db = MySQLdb.connect('localhost', dbuser, dbpass, dbname, charset='utf8')
    cursor = db.cursor()
    sql = 'select * from nebulas_transaction_db where height>=%s and height<=%s' % (
        start_block, end_block)

    try:
        cursor.execute(sql)
        results = cursor.fetchall()
        return results
    except Exception, e:
        print e
    return tuple()


def build_arango_graph(db, txs):
    '''
    usage - build arango transaction graph
    @db - arango client database instance
    @txs - transactions returned from mysql database
    @return - None
    '''

    assert isinstance(db, arango.database.StandardDatabase)
    assert isinstance(txs, tuple)

    collection_height = db.collection('height')
    collection_address = db.collection('address')
    collection_transaction = db.collection('transaction')

    edge_height_next = db.collection('height_next')
    edge_height_txs = db.collection('height_txs')
    edge_from_txs = db.collection('from_txs')
    edge_txs_to = db.collection('txs_to')

    for tx in txs:
        source = tx[4]
        target = tx[9]
        height = tx[15]

        if not collection_height.has(str(height)):
            collection_height.insert({
                '_key': str(height),
                'block_height': height
            })

        if not collection_address.has(source):
            collection_address.insert({'_key': source, 'address': source})
        if not collection_address.has(target):
            collection_address.insert({'_key': target, 'address': target})

        d = {
            '_key': str(tx[0]),
            'nonce': tx[1],
            'status': tx[2],
            'chainId': tx[3],
            'from': tx[4],
            'timestamp': tx[5],
            'gas_used': tx[6],
            'value': tx[7],
            'data': tx[8],
            'to': tx[9],
            'contract_address': tx[10],
            'hash': tx[11],
            'gas_price': tx[12],
            'type': tx[13],
            'gas_limit': tx[14],
            'height': tx[15],
            'type_from': tx[16],
            'type_to': tx[17]
        }
        if not collection_transaction.has(str(tx[0])):
            collection_transaction.insert(d)

        _key = str(height - 1) + '-' + str(height)
        if collection_height.has(str(height - 1)) and collection_height.has(
                str(height)) and not edge_height_next.has(_key):
            edge_height_next.insert({
                '_key': _key,
                '_from': 'height/' + str(height - 1),
                '_to': 'height/' + str(height)
            })
        _key = str(height) + '-' + str(height + 1)
        if collection_height.has(str(height)) and collection_height.has(
                str(height + 1)) and not edge_height_next.has(_key):
            edge_height_next.insert({
                '_key': _key,
                '_from': 'height/' + str(height),
                '_to': 'height/' + str(height + 1)
            })

        _key = str(tx[0])
        if not edge_height_txs.has(_key):
            edge_height_txs.insert({
                '_key': _key,
                '_from': 'height/' + str(height),
                '_to': 'transaction/' + _key
            })

        if not edge_from_txs.has(_key):
            edge_from_txs.insert({
                '_key': _key,
                '_from': 'address/' + source,
                '_to': 'transaction/' + _key
            })

        if not edge_txs_to.has(_key):
            edge_txs_to.insert({
                '_key': _key,
                '_from': 'transaction/' + _key,
                '_to': 'address/' + target
            })

        if height % 10 == 0:
            LOG.info('done with height %d', height)
    return


def get_collection_docs(db, collection):
    '''
    usage - get database specific collection all documents
    @db - arango client database instance
    @collection - collection name
    @return - documents list
    '''

    assert isinstance(db, arango.database.StandardDatabase)
    assert isinstance(collection, str)

    l = list()
    if not db.has_collection(collection):
        return l
    cursor = db.collection(collection).all()
    while cursor.has_more():
        cursor.fetch()
    l = cursor.batch()
    return l


def get_graph_transactions(db):
    '''
    usage - get transaction all collections with graph traverse
    @db - arango client database instance
    @return - documents list
    '''

    assert isinstance(db, arango.database.StandardDatabase)

    l = list()
    if not db.has_graph('txs_graph') or not db.has_collection('height'):
        return l

    graph = db.graph('txs_graph')
    collection_height = db.collection('height')

    for height in collection_height:
        # print height
        result = graph.traverse(
            start_vertex=height,
            direction='outbound',
            strategy='bfs',
            max_depth=1)

        paths = result['paths']
        for path in paths:
            edges = path['edges']
            for edge in edges:
                _from = edge['_from']
                _to = edge['_to']
                if _from.startswith('height/') and _to.startswith(
                        'transaction/'):
                    # print _from, _to
                    vertex_tx = graph.vertex(_to)
                    l.append(vertex_tx)
    return l


def main():
    '''
    usage - func main
    '''
    clear_db(database)
    # drop_db(database)
    # create_db(database)


if __name__ == '__main__':
    main()
