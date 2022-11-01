
ALL = server client benchmark encryption_benchmark proxy clients

all: $(ALL) constants.h

CPPFLAGS = --std=c++11 -g


client: gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o client.o clientHelper.o
	g++ $(CPPFLAGS) client.cpp clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o -lboost_filesystem -lboost_serialization -lthrift -lsodium -pthread -fPIC -o client

clients: clients.cpp gen-cpp/Send_Op.o gen-cpp/Operation_types.o
	g++ $(CPPFLAGS) $^ -lboost_filesystem -lboost_serialization -lthrift -lsodium -pthread -fPIC -o $@

proxy: gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/Send_Op.o gen-cpp/Operation_types.o proxy.o clientHelper.o
	g++ $(CPPFLAGS) $^ -lboost_filesystem -lboost_serialization -lthrift -lsodium -pthread -fPIC -o $@

benchmark: benchmark.cpp clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o
	g++ $(CPPFLAGS) benchmark.cpp clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o -lboost_filesystem -lboost_serialization -lthrift -lsodium -pthread -fPIC -o benchmark

encryption_benchmark: estimate_encryption.cpp clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o
	g++ $(CPPFLAGS) estimate_encryption.cpp clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o -lboost_filesystem -lboost_serialization -lthrift -lsodium -pthread -fPIC -o encryption_benchmark

clientHelper.o: clientHelper.h clientHelper.cpp
	g++ $(CPPFLAGS) clientHelper.cpp -c -fPIC

server: server.cpp gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o
	g++ $(CPPFLAGS) $^ -lthrift -lsodium -lrocksdb -pthread -fPIC -o $@

gen-cpp/KV_RPC.o: gen-cpp/KV_RPC.h gen-cpp/KV_RPC.cpp
	g++ $(CPPFLAGS) gen-cpp/KV_RPC.cpp -c -fPIC -o gen-cpp/KV_RPC.o

gen-cpp/KV_RPC_types.o: gen-cpp/KV_RPC_types.h gen-cpp/KV_RPC_types.cpp
	g++ $(CPPFLAGS) gen-cpp/KV_RPC_types.cpp -c -fPIC -o gen-cpp/KV_RPC_types.o


gen-cpp/KV_RPC.h:
	thrift -r --gen cpp KV_RPC.thrift

gen-cpp/KV_RPC.cpp:
	thrift -r --gen cpp KV_RPC.thrift

gen-cpp/KV_RPC_types.h:
	thrift -r --gen cpp KV_RPC.thrift

gen-cpp/KV_RPC_types.cpp:
	thrift -r --gen cpp KV_RPC.thrift

gen-cpp/Operation_types.cpp:
	thrift -r --gen cpp Operation.thrift

gen-cpp/Operation_types.h:
	thrift -r --gen cpp Operation.thrift

gen-cpp/Send_Op.cpp:
	thrift -r --gen cpp Operation.thrift

gen-cpp/Send_Op.h:
	thrift -r --gen cpp Operation.thrift

clean:
	rm $(ALL) OpScure.data *.o db/* gen-cpp/*

cleandb:
	rm db/* OpScure.data
