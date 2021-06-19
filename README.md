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
## CLI Client
```
$ ./client
> PUT hello world
> GET hello
world
> 
```
Ctrl+C cleans up the program, saves what needs to be persisted into `OpScure.data`, and exits.
