#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/11/14
'''

import sys
sys.path.append('../')
import json
import urllib2


def get_batch_results_by_cursor(dbname, id):
    '''
    @dbname - nebulas or ethereum database name
    @id - cursor returned id last batch
    @return - current batch of results
    '''

    assert isinstance(dbname, str)
    assert isinstance(id, str)

    url = 'http://localhost:8529/_db/' + dbname + '/_api/cursor/' + id
    payload = json.dumps(dict())

    request = urllib2.Request(url, payload)
    request.add_header('Content-Type', 'application/json')
    request.get_method = lambda:'PUT'

    request = urllib2.urlopen(request)

    d = json.loads(request.read())
    d['has_more'] = d['hasMore']
    return json.dumps(d)


def main():
    ret = get_batch_results_by_cursor('nebulas', sys.argv[1])
    print ret


if __name__ == '__main__':
    main()
