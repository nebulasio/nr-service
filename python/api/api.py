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

import db_reader
import nebserver

app = Flask(__name__)
api = Api(app)

apiserver = nebserver.apiserver(__name__, 1024000)


@app.route('/nr')
def api_nr():
    s = str()
    if 'start_block' in request.args and 'end_block' in request.args:
        start_block = int(request.args['start_block'])
        end_block = int(request.args['end_block'])
        s = apiserver.on_api_transaction(start_block, end_block)
    return s


@app.route('/hello')
def api_hello():
    return 'hello\n'


if __name__ == '__main__':
    app.run(host='127.0.0.1', port='6000', debug=True)
