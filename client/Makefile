# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

include ../config.mk

all: build

build: ../gen-cpp/RPC.o ../gen-cpp/Operation_types.o ../constants/constants.h
	$(CXX) -g $(CXXFLAGS) $^ -std=c++17 client.cpp ../crypto/encryption_engine.cpp -lcrypto -lthrift -lsodium -fPIC -o client

clean:
	rm -f *.o client
