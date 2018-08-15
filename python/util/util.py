#!/usr/local/bin/python2.7
'''
Author: Chenmin Wang
Date: 2018/08/13
'''

import logging


def get_logger():
    '''
    usage - log title, level, formatter
    @return - this logger
    '''

    logger = logging.getLogger('transaction graph')
    logger.setLevel(logging.DEBUG)

    # create Stream Handler.
    handler = logging.StreamHandler()
    handler.setLevel(logging.DEBUG)
    # create formatter and add it to the handlers
    formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)
    # add the handlers to the logger
    logger.addHandler(handler)
    return logger


LOG = get_logger()
