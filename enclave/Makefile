# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

include ../config.mk

ifeq ($(OE_CRYPTO_LIB),openssl_3)
	CFLAGS=$(shell pkg-config oeenclave-$(C_COMPILER) --variable=${OE_CRYPTO_LIB}flags)
	CXXFLAGS=$(shell pkg-config oeenclave-$(CXX_COMPILER) --variable=${OE_CRYPTO_LIB}flags)
else
	CFLAGS=$(shell pkg-config oeenclave-$(C_COMPILER) --cflags)
	CXXFLAGS=$(shell pkg-config oeenclave-$(CXX_COMPILER) --cflags)
endif
LDFLAGS=$(shell pkg-config oeenclave-$(CXX_COMPILER) --libs)
INCDIR=$(shell pkg-config oeenclave-$(C_COMPILER) --variable=includedir)
CRYPTO_LDFLAGS=$(shell pkg-config oeenclave-$(COMPILER) --variable=${OE_CRYPTO_LIB}libs)

CRYPTO_SRC = ../crypto
CXXINCDIR = -I. -I../ -I../..
CXXSRCS = common/ecalls.cpp \
		../crypto/encryption_engine.cpp 

# Cover openssl, openssl_symcrypt_fips, and openssl_3
ifneq (,$(findstring openssl,$(OE_CRYPTO_LIB)))
	CRYPTO_SRC = openssl_src
endif

all:
	$(MAKE) build
	$(MAKE) keys
	$(MAKE) sign

build:
	@ echo "Compilers used: $(CC), $(CXX)"
	oeedger8r ../ortoa.edl --trusted \
		--search-path $(INCDIR) \
		--search-path $(INCDIR)/openenclave/edl/sgx
	$(CXX) -g -c $(CXXFLAGS) -DOE_API_VERSION=2 -std=c++11 $(CXXINCDIR) \
		$(CXXSRCS)
	$(CC) -g -c $(CFLAGS) -DOE_API_VERSION=2 ortoa_t.c -o ortoa_t.o
	$(CXX) -o ortoa-enc ecalls.o encryption_engine.o ortoa_t.o $(LDFLAGS) $(CRYPTO_LDFLAGS)

sign:
	oesign sign -e ortoa-enc -c common/ortoa.conf -k private.pem

clean:
	rm -f ortoa-enc ortoa-enc.signed *.o ortoa_t.* ortoa_args.h private.pem public.pem

keys:
	openssl genrsa -out private.pem -3 3072
	openssl rsa -in private.pem -pubout -out public.pem


