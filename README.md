# OpScure
A system to obscure the operation used in KV Store queries.

## Dependencies
- Apache Thrift
- RocksDB
- LibSodium
- Boost

Install all:
```
sudo apt install thrift-compiler libthrift-dev librocksdb-dev libsodium-dev libboost-all-dev
```

## Building
```
$ mkdir db
$ make
```
## Server
```
./server
```
will start the server.
## CLI Client
```
$ ./client
> PUT hello world
> GET hello
world
> 
```
Ctrl+C cleans up the program, saves what needs to be persisted into `OpScure.data`, and exits.

## Shared Library for Java Client
Building the program will output `libwaffle.so`.

## Running Client from YCSB
First put the shared library in a reasonable folder. In order to ensure that it is found by Java, the folder must be added to the environment variable `LD_LIBRARY_PATH`. If the library is contained in `some/folder`, then run the following:
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/some/folder
```
Ensure the server is started, then run the following from the YCSB repository:
```
./bin/ycsb load waffle -s -P workloads/workloada
./bin/ycsb run waffle -s -P workloads/workloada
./bin/ycsb run waffle -s -P workloads/workloadb
./bin/ycsb run waffle -s -P workloads/workloadc
```
Loading only needs to be done once. Then check the `logs` folder in the YCSB repository for throughput and latency numbers (`logs/loga.txt` corresponds to workload A, and so on).
