#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/11/13
'''

import sys
sys.path.append('../')
import os
from util.util import LOG

from arango import ArangoClient


dbuser = os.environ['DB_USER_NAME']
dbpass = os.environ['DB_PASSWORD']

client = ArangoClient(protocol='http', host='localhost', port=8529)
nas_db = client.db(username=dbuser, password=dbpass, name=os.environ['NEBULAS_DB'])
eth_db = client.db(username=dbuser, password=dbpass, name=os.environ['ETH_DB'])

db_client = {
    'nebulas': nas_db,
    'eth': eth_db
}
