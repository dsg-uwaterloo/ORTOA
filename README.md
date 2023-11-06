# ORTOA-TEE

## Project Dependencies

| Name             | Installation                                                                                                                               |
| ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| `clang-format`   | `sudo apt-get install clang-format`                                                                                                        |
| Open Enclave SDK | [OpenEnclave documentation](https://github.com/openenclave/openenclave/blob/master/docs/GettingStartedDocs/install_oe_sdk-Ubuntu_20.04.md) |

## Python Packages

Our package requires `python>=3.8.*` and can be initialized via `pip`:

```bash
ORTOA/ $ python3 -m venv .venv
ORTOA/ $ . .venv/bin/activate
ORTOA/ $ pip install -e extras/
```

The dev dependencies can be installed via `pip` as well:

```bash
ORTOA/ $ pip install -e extras/[dev]
```

## Sourcing Scripts & Utilities

```bash
ORTOA/ $ source scripts/ortoa-lib.sh
```

## Building the codebase

```bash
# Create build directory
ORTOA/ $ mkdir build && cd build

# Build
ORTOA/build/ $ cmake ..
ORTOA/build/ $ make
```

## Running ORTOA

### After building the codebase & sourcing the scripts...

#### `ortoa-simulate`

```bash
ORTOA/ $ ortoa-simulate -h
```

#### `ortoa-client-run`

```bash
ORTOA/ $ ortoa-client-run -h
```

# Previously in `dependencies.txt`

```bash
git clone https://github.com/redis/hiredis.git
cd hiredis
make
sudo make install
cd ..
git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus
mkdir build
cd build/
cmake -DREDIS_PLUS_PLUS_CXX_STANDARD=17 ..
make
make install
sudo make install
```
