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

apiserver = nebserver.apiserver(__name__, 1 << 20)


@app.route('/nr')
def api_nr():
    return apiserver.on_api_transaction(request.args.to_dict())


@app.route('/hello')
def api_hello():
    return 'hello\n'


if __name__ == '__main__':
    app.run(host='127.0.0.1', port='6000', debug=True)
