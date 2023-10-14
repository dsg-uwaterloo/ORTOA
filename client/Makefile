# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

include ../config.mk

all: build

build: ../gen-cpp/RPC.o ../gen-cpp/Operation_types.o ../constants/network.h
	$(CXX) -g $(CXXFLAGS) $^ -std=c++17 client.cpp -lboost_filesystem -lboost_serialization -lthrift -lsodium -pthread -fPIC -o client
	# $(CXX) -g $(CXXFLAGS) -std=c++17 client.cpp ../host/redis.cpp ../crypto/encryption_engine.cpp \
	# 					 /usr/local/lib/libredis++.a /usr/local/lib/libhiredis.a -lcrypto -lssl -lthrift -o client

clean:
	rm -f *.o client
