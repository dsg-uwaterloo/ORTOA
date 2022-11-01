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

# Configuring system
You can configure many system parameters by modifying ```constants.h```. This file controls ports, db size, value size, and IP addresses. Note that you will have to recompile each executable after modifying the constants file.


## Building
```
$ mkdir db
$ make
```
## Server
```
./server
```
will start the server. The server will run until a value fetch fails or until you quit it.

#clients.cpp

```
$ make clients
$ ./clients
```
By default, this will run with 64 threads, each making 100 requests to the proxy with equal probability of a get and put request. These values can all be changed by editing the clients.cpp file.

## CLI Client
```
$ ./client
> PUT hello world
> GET hello
world
> 
```
Ctrl+C cleans up the program, saves what needs to be persisted into `OpScure.data`, and exits. EXIT will also quit the program.
