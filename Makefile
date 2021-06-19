
ALL = server client

all: $(ALL)

libOpscureJavaClient.so: OpscureJavaClient.h OpscureJavaClient.cpp clientHelper.h clientHelper.o
	$(CXX) $(CPPFLAGS) -shared -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux -o libOpscureJavaClient.so OpscureJavaClient.cpp clientHelper.o -l -fPIC

client: client.cpp clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o
	g++ client.cpp clientHelper.o gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o -lthrift -lsodium -fPIC -o client

clientHelper.o: clientHelper.h clientHelper.cpp
	g++ clientHelper.cpp -c

server: server.cpp gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o
	g++ server.cpp gen-cpp/KV_RPC.o gen-cpp/KV_RPC_types.o gen-cpp/KV_RPC_constants.o -lthrift -lsodium -lrocksdb -fPIC -o server

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
	rm *.o db/* gen-cpp/*
