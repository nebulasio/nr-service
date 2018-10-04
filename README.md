# Nr-service

Nebulas rank offline service

## Nebulas node dependency

Compile and install nebulas, tutorial [here](https://github.com/nebulasio/wiki/blob/master/tutorials/%5BEnglish%5D%20Nebulas%20101%20-%2001%20Installation.md).

## Install dependencies (>=Ubunut16.04)

### Development environment dependency

  - Cpp boost library, Boost version >= 1.67.0
  - Compiler, LLVM/Clang++ version >= 6.0.0
  - Python version 2.7
  - Python restful api framework, Flask version >= 1.0.2

### Arangodb dependency

  - Arangodb version >= 3.3.14
  - Arangodb c++ driver fuerte, branch [makeFuerteGreateAgain](https://github.com/arangodb/fuerte/tree/makeFuerteGreatAgain)
  - Arangodb serialization and storage format, [velocypack](https://github.com/arangodb/velocypack)
  - Arangodb python driver, [python-arango](https://github.com/joowani/python-arango)
  - Http-parser version >= 2.8.1
  
### Install with script

```sh
$ git clone git@github.com:nebulasio/nr-service.git
$ cd nr-service/cpp
$ ./prepare.sh
```

When running with *prepare.sh*, it will take a long time to compile, build and install depencencies, just be patient.

### Install arangodb

```sh
$ sudo dpkg -i 3rd_party/arangodb3-3.3.16-1_amd64.deb
```

### Install arangodb python driver

```sh
$ sudo pip install python-arango
```

### Install flask

```sh
$ sudo pip install Flask
$ sudo pip install flask-restful
```

### Import environment variables

```sh
$ source env.set.sh
```

## Build

```sh
$ cd /path/to/nr-service
$ mkdir build && cd build
$ cmake .. && make
```

## Run

### Run nebulas node

```sh
$ cd path/to/go-nebulas
$ ./neb -c mainnet/conf/config.conf
```
Wait until nebulas node syncs blocks from genesis block to the latest, it takes days or week.

### Run arangodb

```sh
$ arangod --server.endpoint http+tcp://127.0.0.1:8529
```

### Build transaction graph

```sh
$ cd path/to/nr-service
$ python python/arango/arango_graph.py
$ ./cpp/bin/transaction_writer
```

 Running `transaction_writer` in directory `$PATH_TO_NR-SERVICE/cpp/bin` will build transaction graph to arangodb colelctions from nebulas node (genesis block to the latest), also takes days or week.
 
 ### Build nr db collection
 
 ```sh
 $ cd path/to/nr-servie
 $ ./cpp/bin/nr --start_ts=xx --end_ts=xx
 ```
 
 Build nr db collection from start timestamp to end timestamp with day interval.
 
 ### Start REST API server
 
 ```sh
 $ cd path/to/nr-service
 $ python python/api/api.py
 ```

## Query

### Query transaction by block height or address
```sh
$ curl -s "http://localhost:6000/nebulas-transaction?start_block=400100&end_block=400200"
$ curl -s "http://localhost:6000/nebulas-transaction?address=n1Q6JhXKWXCkyvoqymN4LPd6J1tentyKRVF"
```

### Query nebulas rank by date
```sh
$ curl -s "http://localhost:6000/nebulas-nr?date=20180620"
```
