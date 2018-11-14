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


app = Flask(__name__)
api = Api(app)


@app.route('/transaction')
def api_nebulas_transaction():
    ret = str()
    if 'address' not in request.args:
        return ret
    if 'db' not in request.args:
        return ret

    address = str(request.args['address'])
    dbname = str(request.args['db'])
    batch_size = 1 << 8

    if 'batch_size' in request.args:
        batch_size = int(request.args['batch_size'])

    return api_transaction.get_transactions_by_address(dbname, batch_size, address)


@app.route('/nr')
def api_nebulas_nr():
    ret = str()
    if 'date' not in request.args:
        return ret
    if 'db' not in request.args:
        return ret

    date = str(request.args['date'])
    dbname = str(request.args['db'])
    batch_size = 1 << 8
    if 'batch_size' in request.args:
        batch_size = int(request.args['batch_size'])

    if 'address' in request.args:
        address = str(request.args['address'])
        return api_nr.get_nr_by_date_address(dbname, batch_size, date, address)

    return api_nr.get_nr_by_date(dbname, batch_size, date)


@app.route('/cursor')
def api_nebulas_cursor():
    ret = str()
    if 'db' not in request.args:
        return ret
    if 'id' not in request.args:
        return ret

    dbname = str(request.args['db'])
    id = str(request.args['id'])
    return  api_cursor.get_batch_results_by_cursor(dbname, id)


if __name__ == '__main__':
    app.run(host='127.0.0.1', port='6003', threaded=True, debug=True)
