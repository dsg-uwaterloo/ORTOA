# ORTOA-TEE

## Dependencies

|      Name      |             Installation            |
| -------------- | ----------------------------------- |
| `clang-format` | `sudo apt-get install clang-format` |
| Open Enclave SDK | [OpenEnclave documentation](https://github.com/openenclave/openenclave/blob/master/docs/GettingStartedDocs/install_oe_sdk-Ubuntu_20.04.md) |


## Sourcing Scripts & Utilities

```bash
smaiyya@tem121:~/sgx/adrian/ORTOA/ $ source scripts/ortoa-lib.sh
```

## Building the codebase

```bash
# Create build directory
smaiyya@tem121:~/sgx/adrian/ORTOA/ $ mkdir build && cd build

# Build
smaiyya@tem121:~/sgx/adrian/ORTOA/build/ $ cmake ..
smaiyya@tem121:~/sgx/adrian/ORTOA/build/ $ make
```

## Running ORTOA

### After building the codebase & sourcing the scripts...

#### `ortoa-simulate`

```bash
smaiyya@tem121:~/sgx/adrian/ORTOA/build/ $ ortoa-simulate -h
```

#### `ortoa-client-run`

```bash
smaiyya@tem121:~/sgx/adrian/ORTOA/build/ $ ortoa-client-run -h
```
