# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

include ../config.mk

CFLAGS=$(shell pkg-config oehost-$(C_COMPILER) --cflags)
CXXFLAGS=$(shell pkg-config oehost-$(CXX_COMPILER) --cflags)
LDFLAGS=$(shell pkg-config oehost-$(CXX_COMPILER) --libs)
INCDIR=$(shell pkg-config oehost-$(C_COMPILER) --variable=includedir)

all: build

build: ../gen-cpp/RPC.o ../gen-cpp/Operation_types.o ../constants/constants.h
	oeedger8r ../ortoa.edl --untrusted \
		--search-path $(INCDIR) \
		--search-path $(INCDIR)/openenclave/edl/sgx
	$(CXX) -g $(CXXFLAGS) $^ -std=c++17 host.cpp redis.cpp ortoa_u.c /usr/local/lib/libredis++.a /usr/local/lib/libhiredis.a $(LDFLAGS) -lrt -lboost_filesystem -lboost_serialization -lthrift -lsodium -pthread -fPIC -o ortoa-host

clean:
	rm -f ortoa-host ortoa_u.* ortoa_args.h *.o ../out.decrypted ../out.encrypted