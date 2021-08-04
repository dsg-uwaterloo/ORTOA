
ALL = server client libwaffle.so

JAVA_HOME = /usr/lib/jvm/java-11-openjdk-amd64

all: $(ALL)

libwaffle.so: waffle.h waffle.cpp clientHelper.h clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o
	$(CXX) $(CPPFLAGS) -shared -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux -o libwaffle.so waffle.cpp clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o -lboost_filesystem -lboost_serialization -lthrift -lsodium -fPIC

client: client.cpp clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o
	g++ client.cpp clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o -lboost_filesystem -lboost_serialization -lthrift -lsodium -pthread -fPIC -o client

clientHelper.o: clientHelper.h clientHelper.cpp
	g++ clientHelper.cpp -c -fPIC

server: server.cpp gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o
	g++ server.cpp gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o -lthrift -lsodium -lrocksdb -pthread -fPIC -o server

gen-cpp/KV_RPC.o: gen-cpp/KV_RPC.h gen-cpp/KV_RPC.cpp
	g++ gen-cpp/KV_RPC.cpp -c -fPIC -o gen-cpp/KV_RPC.o

gen-cpp/KV_RPC_types.o: gen-cpp/KV_RPC_types.h gen-cpp/KV_RPC_types.cpp
	g++ gen-cpp/KV_RPC_types.cpp -c -fPIC -o gen-cpp/KV_RPC_types.o

gen-cpp/KV_RPC_constants.o: gen-cpp/KV_RPC_constants.h gen-cpp/KV_RPC_constants.cpp
	g++ gen-cpp/KV_RPC_constants.cpp -c -fPIC -o gen-cpp/KV_RPC_constants.o

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
	rm $(ALL) OpScure.data *.o db/* gen-cpp/*

cleandb:
	rm db/* OpScure.data
