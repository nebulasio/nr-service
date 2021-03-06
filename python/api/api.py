#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/08/24
'''

import sys
sys.path.append('../')
sys.path.append('../../cpp/build')

from flask import Flask, url_for, request, json
from flask_restful import Resource, Api

import json
import db_reader
import nebserver

app = Flask(__name__)
api = Api(app)

nebulas_transaction_apiserver = nebserver.nebulas_transaction_apiserver(__name__)
nebulas_account_apiserver = nebserver.nebulas_account_apiserver(__name__)
nebulas_nr_apiserver = nebserver.nebulas_nr_apiserver(__name__)

eth_transaction_apiserver = nebserver.eth_transaction_apiserver(__name__)
eth_account_apiserver = nebserver.eth_account_apiserver(__name__)
eth_nr_apiserver = nebserver.eth_nr_apiserver(__name__)


@app.route('/nebulas-transaction')
def api_nebulas_transaction():
    return nebulas_transaction_apiserver.on_api_transaction(request.args.to_dict())


@app.route('/nebulas-account')
def api_nebulas_account():
    return nebulas_account_apiserver.on_api_account(request.args.to_dict())


@app.route('/nebulas-nr')
def api_nebulas_nr():
    return nebulas_nr_apiserver.on_api_nr(request.args.to_dict())


@app.route('/eth-transaction')
def api_eth_transaction():
    return eth_transaction_apiserver.on_api_transaction(request.args.to_dict())


@app.route('/eth-account')
def api_eth_account():
    return eth_account_apiserver.on_api_account(request.args.to_dict())


@app.route('/eth-nr')
def api_eth_nr():
    return eth_nr_apiserver.on_api_nr(request.args.to_dict())


@app.route('/hello')
def api_hello():
    return 'hello\n'


if __name__ == '__main__':
    app.run(host='127.0.0.1', port='6000', threaded=True, debug=True)
