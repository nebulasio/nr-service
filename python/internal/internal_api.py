#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/11/13
'''

import sys
sys.path.append('../')

from flask import Flask, url_for, request, json
from flask_restful import Resource, Api

import api_transaction
import api_nr
import api_cursor
import api_keyset


app = Flask(__name__)
api = Api(app)


@app.route('/transaction')
def api_nebulas_transaction():
    if 'address' not in request.args:
        return str()
    if 'db' not in request.args:
        return str()

    address = str(request.args['address'])
    dbname = str(request.args['db'])
    batch_size = 1 << 8

    if 'batch_size' in request.args:
        batch_size = int(request.args['batch_size'])

    start_ts = str()
    end_ts = str()

    if 'start_ts' in request.args and 'end_ts' in request.args:
        start_ts = str(request.args['start_ts'])
        end_ts = str(request.args['end_ts'])

    return api_transaction.get_transactions_by_address(dbname, batch_size, address, start_ts, end_ts)


@app.route('/nr')
def api_nebulas_nr():
    if 'db' not in request.args:
        return str()
    if 'date' not in request.args and 'address' not in request.args:
        return str()

    dbname = str(request.args['db'])
    batch_size = 1 << 8
    if 'batch_size' in request.args:
        batch_size = int(request.args['batch_size'])

    collection = 'nr'
    if 'collection' in request.args:
        collection = str(request.args['collection'])

    if 'date' in request.args and 'address' in request.args:
        date = str(request.args['date'])
        address = str(request.args['address'])
        return api_nr.get_nr_by_date_address(dbname, batch_size, date, address, collection)
    elif 'date' in request.args:
        date = str(request.args['date'])
        return api_nr.get_nr_by_date(dbname, batch_size, date, collection)
    elif 'address' in request.args:
        address = str(request.args['address'])
        return api_nr.get_nr_by_address(dbname, batch_size, address, collection)

    return str()


@app.route('/keyset')
def api_nebulas_key():
    if 'db' not in request.args:
        return str()
    if 'collection' not in request.args:
        return str()
    if 'field' not in request.args:
        return str()

    dbname = str(request.args['db'])
    collection = str(request.args['collection'])
    field = str(request.args['field'])

    batch_size = 1 << 8
    if 'batch_size' in request.args:
        batch_size = int(request.args['batch_size'])

    return api_keyset.get_keys(dbname, collection, batch_size, field)


@app.route('/cursor')
def api_nebulas_cursor():
    if 'db' not in request.args:
        return str()
    if 'id' not in request.args:
        return str()

    dbname = str(request.args['db'])
    id = str(request.args['id'])
    return  api_cursor.get_batch_results_by_cursor(dbname, id)


if __name__ == '__main__':
    app.run(host='127.0.0.1', port='6003', threaded=True, debug=True)
