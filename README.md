# ORTOA-TEE

ORTOA- a One Round Trip Oblivious Access protocol that reads or writes data stored on remote storage *in one round without revealing the type of access*.

The ORTOA-TEE project implements this access protocol leveraging the cryptographic guarantees of trusted execution environments (hardware enclaves) and benchmarks the performance.

## Scripts & Utilities

We provide a bash script library `srcipts/ortoa-lib.sh` that houses a collection of scripts and utilities to manage the `ORTOA` environment. It can be sourced as follows:

```bash
ORTOA/ $ source scripts/ortoa-lib.sh
```

## Building the codebase

Details about the build can be found at [TODO](todo). The easiest way to build the C++ projects is to run the `ortoa-cbi` script.

```bash
ORTOA/ $ ortoa-cbi  # requires sourcing scripts & utilities
```

## Running ORTOA


### `ortoa-client-run`

Runs the `ORTOA-TEE` client

```bash
ORTOA/ $ ortoa-client-run -h
```

### `ortoa-host`

Runs the `ORTOA-TEE` host. Requires SGX hardware.

```bash
ORTOA/ $ ortoa-host -h
```

### `ortoa-simulate`

Runs the `ORTOA-TEE` host in simulation mode.

```bash
ORTOA/ $ ortoa-simulate -h
```

