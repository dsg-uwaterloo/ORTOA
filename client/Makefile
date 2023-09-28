# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

include ../config.mk

all: build

build:
	$(CXX) -g $(CXXFLAGS) -std=c++17 client.cpp ../host/redis.cpp ../crypto/encryption_engine.cpp \
						 /usr/local/lib/libredis++.a /usr/local/lib/libhiredis.a -lcrypto -lssl -o client

clean:
	rm -f *.o client
