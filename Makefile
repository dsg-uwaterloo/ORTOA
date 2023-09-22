# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

.PHONY: all build clean run simulate

OE_CRYPTO_LIB := openssl
export OE_CRYPTO_LIB

all: build

build:
	$(MAKE) -C enclave
	$(MAKE) -C host
	$(MAKE) -C client

clean:
	$(MAKE) -C enclave clean
	$(MAKE) -C host clean
	$(MAKE) -C client clean

run:
	host/ortoa-host ./enclave/ortoa-enc.signed

simulate:
	host/ortoa-host ./enclave/ortoa-enc.signed --simulate

client-run:
	client/client

