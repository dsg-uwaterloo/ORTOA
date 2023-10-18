// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <assert.h>
#include <iostream>
#include <openenclave/host.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "ortoa_u.h"
#include "redis.h"
#include "../constants/constants.h"
#include "../constants/shared.h"
#include "../gen-cpp/RPC.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

bool check_simulate(int argc, char *argv[]) {
  for (int i = 2; i < argc; ++i) {
    if (strcmp(argv[i], "--simulate") == 0) return true;
  }
  return false;
}

class RPCHandler : virtual public RPCIf {
 private:
  inline static uint32_t simulate_flag = OE_ENCLAVE_FLAG_DEBUG;
  inline static char* oe_enclave_path;
  inline static oe_enclave_t* enclave;
  redisCli rd;

 public:
  RPCHandler() {
    oe_result_t result = oe_create_ortoa_enclave(oe_enclave_path, OE_ENCLAVE_TYPE_SGX, simulate_flag, NULL, 0, &enclave);
    if (result != OE_OK) {
      std::cerr << "oe_create_ortoa_enclave() failed with enclave path " << oe_enclave_path << " " << result << std::endl;
      throw "oe ortoa enclave init failed";
    }
  }

  static void setEnclaveArgs(int argc, char *argv[]) {
    assert(argc >= 2);

    oe_enclave_path = argv[1];
    if (check_simulate(argc, argv)) {
      std::cout << "Running in simulation mode" << std::endl;
      RPCHandler::simulate_flag = OE_ENCLAVE_FLAG_SIMULATE;
    }
  }

  void access(std::string& _return, const Operation& operation) {
    std::string key = operation.key;
    std::string val = rd.get(key);
    std::string update_val = operation.value;

    std::unique_ptr<unsigned char> out(new unsigned char[4096]);
    size_t out_len;
    oe_result_t result = access_data(enclave, operation.op, val.c_str(), val.length(), update_val.c_str(), update_val.length(), out.get(), &out_len);
    if (result == OE_OK) {
      std::string updated_val((const char *) out.get(), out_len);
      std::cout << "[Host]: Output of access_data " << updated_val << " with len " << out_len << std::endl;
      rd.put(key, updated_val);
    }
  }
};

int main(int argc, char *argv[]) {
  RPCHandler::setEnclaveArgs(argc, argv);

  auto handler = std::make_shared<RPCHandler>();
  auto processor = std::make_shared<RPCProcessor>(handler);
  auto serverTransport = std::make_shared<TServerSocket>(HOST_PORT);
  auto transportFactory = std::make_shared<TBufferedTransportFactory>();
  auto protocolFactory = std::make_shared<TBinaryProtocolFactory>();

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}
