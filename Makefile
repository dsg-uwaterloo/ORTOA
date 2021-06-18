
ALL = server client

all: $(ALL)

proxy: proxy.cpp
	g++ proxy.cpp -lsodium -o proxy

client: client.cpp gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o
	g++ client.cpp gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o -lthrift -lsodium -o client

server: server.cpp gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o
	g++ server.cpp gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o -lthrift -lsodium -lrocksdb -o server

gen-cpp/KV_RPC.o: gen-cpp/KV_RPC.h gen-cpp/KV_RPC.cpp
	g++ gen-cpp/KV_RPC.cpp -c -o gen-cpp/KV_RPC.o

gen-cpp/KV_RPC_types.o: gen-cpp/KV_RPC_types.h gen-cpp/KV_RPC_types.cpp
	g++ gen-cpp/KV_RPC_types.cpp -c -o gen-cpp/KV_RPC_types.o

gen-cpp/KV_RPC_constants.o: gen-cpp/KV_RPC_constants.h gen-cpp/KV_RPC_constants.cpp
	g++ gen-cpp/KV_RPC_constants.cpp -c -o gen-cpp/KV_RPC_constants.o

gen-cpp/KV_RPC.h:
	thrift -r --gen cpp KV_RPC.thrift

gen-cpp/KV_RPC.cpp:
	thrift -r --gen cpp KV_RPC.thrift

gen-cpp/KV_RPC_types.h:
	thrift -r --gen cpp KV_RPC.thrift

gen-cpp/KV_RPC_types.cpp:
	thrift -r --gen cpp KV_RPC.thrift

gen-cpp/KV_RPC_constants.h:
	thrift -r --gen cpp KV_RPC.thrift

gen-cpp/KV_RPC_constants.cpp:
	thrift -r --gen cpp KV_RPC.thrift

clean:
	rm *.o db/* gen-cpp/*
