# ORTOA-TEE

## Dependencies

|      Name      |             Installation            |
| -------------- | ----------------------------------- |
| `clang-format` | `sudo apt-get install clang-format` |


## Building the codebase

```bash
# Create build directory
smaiyya@tem121:~/sgx/adrian/ORTOA/ $ mkdir build && cd build

# Build
smaiyya@tem121:~/sgx/adrian/ORTOA/build/ $ cmake ..
smaiyya@tem121:~/sgx/adrian/ORTOA/build/ $ make
```

## Available targets

### After building the codebase...

#### `make simulate`

```bash
smaiyya@tem121:~/sgx/adrian/ORTOA/build/ $ make simulate
```

#### `make client-run`

```bash
smaiyya@tem121:~/sgx/adrian/ORTOA/build/ $ make client-run
```

---
---
---

# Old README:

Install openenclave sdk:
https://github.com/openenclave/openenclave/blob/master/docs/GettingStartedDocs/install_oe_sdk-Ubuntu_20.04.md

Execute the steps in dependencies.txt to install Redis.

Then run the following command to build the codebase:

make build

Run the following command to run the client (which for now inserts 2 keys):

make client-run

Run the following command to run ORTOA code in simulation mode:

make simulate
