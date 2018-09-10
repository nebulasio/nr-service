# Nr-service

Nebulas rank offline service

# Nebulas node dependency

Compile and install nebulas, tutorial [here](https://github.com/nebulasio/wiki/blob/master/tutorials/%5BEnglish%5D%20Nebulas%20101%20-%2001%20Installation.md).

# Development environment dependency

  - Boost version >= 1.67.0
  - LLVM/Clang++ version >= 6.0.0
  - Python version 2.7
  - Flask version >= 1.0.2

# Arangodb dependency

  - Arangodb version >= 3.3.14
  - Arangodb c++ driver fuerte, branch [makeFuerteGreateAgain](https://github.com/arangodb/fuerte/tree/makeFuerteGreatAgain)
  - Arangodb serialization and storage format, [velocypack](https://github.com/arangodb/velocypack)
  - Arangodb python driver, [python-arango](https://github.com/joowani/python-arango)
  - Http-parser version >= 2.8.1 

# Build

```sh
$ cd /path/to/nr-service
$ mkdir build && cd build
$ cmake .. && make
```

# Run

  - Run nebulas node, sync blocks from genesis block to the lastest
  - Run arangodb, build arangodb collections, script in `$PATH_TO_NR-SERVICE/python/graph/arango_graph.py`
  - Run `transaction_writer` and `account_writer` in directory `$PATH_TO_NR-SERVICE/cpp/bin`, to build transaction graph
  - Run `nr` in the same directory

# Query

  - Query transaction by block height, `curl -s "http://localhost:6000/transaction?start_block=400100&end_block=400200"`
  - Query transaction by address, `curl -s "http://localhost:6000/transaction?address=n1Q6JhXKWXCkyvoqymN4LPd6J1tentyKRVF"`
  - Query account by address, `curl -s "http://localhost:6000/account?address=n1Q6JhXKWXCkyvoqymN4LPd6J1tentyKRVF"`
  - Query nebulas rank by date, `curl -s "http://localhost:6000/nr?date=20180620"`
