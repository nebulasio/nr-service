#!/usr/bin/python3.5
'''
Author: Chenmin Wang
Date: 2018/08/28
'''

import sys
sys.path.append('../../cpp/build')
import nebserver

apiserver = nebserver.apiserver(__name__)


def main():
    start_block = int(sys.argv[1])
    end_block = int(sys.argv[2])
    apiserver.on_api_transaction(start_block, end_block)
    return


if __name__ == '__main__':
    main()
