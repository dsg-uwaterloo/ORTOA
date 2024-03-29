# ORTOA-TEE Installation

## Installing the C++ project

### 1. OpenEnclave SDK

Follow the steps listed below to install OpenEnclave SDK. More detailed installation instructions can be found in the [OpenEnclave documentation](https://github.com/openenclave/openenclave/blob/master/docs/GettingStartedDocs/install_oe_sdk-Ubuntu_20.04.md).

First, configure the Intel and Microsoft APT Repositories:
```bash
echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu focal main' | sudo tee /etc/apt/sources.list.d/intel-sgx.list
wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | sudo apt-key add -

echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-11 main" | sudo tee /etc/apt/sources.list.d/llvm-toolchain-focal-11.list
wget -qO - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -

echo "deb [arch=amd64] https://packages.microsoft.com/ubuntu/20.04/prod focal main" | sudo tee /etc/apt/sources.list.d/msprod.list
wget -qO - https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -

sudo apt update
```

Then, install the Intel and Open Enclave packages and dependencies:
```bash
sudo apt -y install clang-11 libssl-dev gdb libsgx-enclave-common libsgx-quote-ex libprotobuf17 libsgx-dcap-ql libsgx-dcap-ql-dev az-dcap-client open-enclave
```

Finally, to make use of the Open Enclave CMake package, please install CMake:
```bash
sudo apt-get install python3-pip
sudo pip3 install cmake
```

### 2. Apache Thrift

First, install the tools and libraries required to build and install the Apache Thrift compiler and C++ libraries. The following instructions are for an Ubuntu Linux based system. Pulled from this [Thrift documentation](https://thrift.apache.org/docs/install/debian.html).

```bash
sudo apt-get install automake bison flex g++ git libboost-all-dev libevent-dev libssl-dev libtool make pkg-config
```

Then, install the Thrift compiler:

```bash
sudo apt install thrift-compiler
```

Finally, install the Thrift C++ library:

```bash
sudo apt install libthrift-dev
```

### 3. Redis C/C++ Client

First, install [`Redis`](https://redis.io/docs/install/install-redis/install-redis-on-linux).
```bash
ORTOA/ $ sudo apt-get install redis
```

Next, install [`hiredis`](https://github.com/redis/hiredis). _(Note: do not install multiple versions of `hiredis`. Otherwise, there might be some bizarre conflicts.)_

```bash
# Clone hiredis
ORTOA/ $ git clone https://github.com/redis/hiredis.git
ORTOA/ $ cd hiredis

# Build hiredis
ORTOA/hiredis $ make
ORTOA/hiredis $ sudo make install
```

Then, install [`redis-plus-plus`](https://github.com/sewenew/redis-plus-plus). _(Since `redis-plus-plus` depends on `hiredis`, ensure that `hiredis` is installed first.)_

```bash
# Clone redis-plus-plus
ORTOA/ $ git clone https://github.com/sewenew/redis-plus-plus.git
ORTOA/ $ cd redis-plus-plus

# Create the build directory
ORTOA/redis-plus-plus $ mkdir build
ORTOA/redis-plus-plus $ cd build

# Build redis-plus-plus
ORTOA/redis-plus-plus/build $ cmake -DREDIS_PLUS_PLUS_CXX_STANDARD=17 ..
ORTOA/redis-plus-plus/build $ make
ORTOA/redis-plus-plus/build $ sudo make install
```

Finally, you can clean up the repo by deleting the `hiredis/` and `redis-plus-plus/` directories. This is because (by default) they are installed at `/usr/local`.

### 4. Sodium

[Sodium](https://github.com/jedisct1/libsodium) is a modern, easy-to-use software library for encryption, decryption, signatures, password hashing, and more. It can be installed with the following command:

```bash
sudo apt-get install -y libsodium-dev
```

### [Optional] Dev Dependencies

| Name           | Installation                        |
| -------------- | ----------------------------------- |
| `clang-format` | `sudo apt-get install clang-format` |

## Installing the Python SDK

### Requirements

```bash
sudo apt install python3.8-venv
```

### Installation

Our package requires `python>=3.8.*` and can be initialized via `pip`:

```bash
# Create & activate a virtual environment
ORTOA/ $ python3 -m venv .venv
ORTOA/ $ . .venv/bin/activate

# Install the package
ORTOA/ $ pip install -e extras/
```

The dev dependencies can be installed via `pip` as well:

```bash
ORTOA/ $ pip install -e extras/[dev]
```
