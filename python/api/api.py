#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/08/24
'''

import sys
sys.path.append('../')

from flask import Flask, url_for, request, json
from flask_restful import Resource, Api

import db_reader

app = Flask(__name__)
api = Api(app)

apiserver = nebserver.apiserver(__name__)


@app.route('/nr')
def api_nr():
    l = list()
    if 'start_block' in request.args and 'end_block' in request.args:
        start_block = int(request.args['start_block'])
        end_block = int(request.args['end_block'])
        l = db_reader.read_from_transaction_with_duration(start_block, end_block)
    return l


@app.route('/hello')
def api_hello():
    return 'hello\n'


if __name__ == '__main__':
    app.run(host='127.0.0.1', port='6000', debug=True)
