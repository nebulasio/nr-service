import sys, os
import logging

import MySQLdb
from arango import ArangoClient

dbuser = os.environ['DB_USER_NAME']
dbpass = os.environ['DB_PASSWORD']
dbname = os.environ['NEBULAS_DB']
client = ArangoClient(protocol='http', host='localhost', port=8529)
db = client.db(username=dbuser, password=dbpass, name=dbname)

collection_list = ['height', 'address', 'transaction']
edge_list = ['height_next', 'height_txs', 'from_txs', 'txs_to']


def get_logger():
    logger = logging.getLogger('transaction graph')
    logger.setLevel(logging.DEBUG)

    # create Stream Handler.
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)
    # create formatter and add it to the handlers
    formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    # add the handlers to the logger
    logger.addHandler(ch)
    return logger


logger = get_logger()


def create_db(db):
    if not db.has_database(dbname):
        db.create_database(dbname)

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

    if not db.has_graph('txs_graph'):
        graph = db.create_graph('txs_graph')
    else:
        graph = db.graph('txs_graph')

    for collection in collection_list:
        if not graph.has_vertex_collection(collection):
            graph.create_vertex_collection(collection)

    if not graph.has_edge_definition('height_next'):
        graph.create_edge_definition(
            edge_collection='height_next',
            from_vertex_collections=['height'],
            to_vertex_collections=['height'])

    if not graph.has_edge_definition('height_txs'):
        graph.create_edge_definition(
            edge_collection='height_txs',
            from_vertex_collections=['height'],
            to_vertex_collections=['transaction'])

    if not graph.has_edge_definition('from_txs'):
        graph.create_edge_definition(
            edge_collection='from_txs',
            from_vertex_collections=['address'],
            to_vertex_collections=['transaction'])

    if not graph.has_edge_definition('txs_to'):
        graph.create_edge_definition(
            edge_collection='txs_to',
            from_vertex_collections=['transaction'],
            to_vertex_collections=['address'])


def drop_db(db):
    if db.has_graph('txs_graph'):
        db.delete_graph('txs_graph')

    for collection in db.collections():
        if collection['system'] is False:
            name = collection['name']
            db.delete_collection(name)
    return


def clear_db(db):
    for collection in db.collections():
        if collection['system'] is False:
            name = collection['name']
            db.collection(name).truncate()
    return


# drop_db(db)
# create_db(db)
# clear_db(db)
# logger.info('clear db')


def get_transactions_from_mysql(start_block, end_block):
    db = MySQLdb.connect('localhost', dbuser, dbpass, dbname, charset='utf8')
    cursor = db.cursor()
    sql = 'select * from nebulas_transaction_db where height>=%s and height<=%s' % (
        start_block, end_block)

    try:
        cursor.execute(sql)
        results = cursor.fetchall()
        return results
    except:
        print 'exception'
    return tuple()


# txs = get_transactions_from_mysql(sys.argv[1], sys.argv[2])
# logger.info('get transaction from mysql')


def build_arango_graph(db, txs):
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
            logger.info('done with height %d' % height)
    return


# build_arango_graph(db, txs)
# logger.info('build arango graph')


def get_collection_docs(db, collection):
    l = list()
    if not db.has_collection(collection):
        return l
    cursor = db.collection(collection).all()
    while cursor.has_more():
        cursor.fetch()
    l = cursor.batch()
    return l


# l = get_collection_docs(db, 'transaction')


def get_graph_transactions(db):
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

#l = get_graph_transactions(db)
