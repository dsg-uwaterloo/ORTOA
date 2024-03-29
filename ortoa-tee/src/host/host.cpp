// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <cassert>
#include <openenclave/host.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include "RPC.h"
#include "constants.h"
#include "errors.h"
#include "ortoa_u.h"
#include "redis.h"
#include "shared.h"
#include "spdlog/spdlog.h"

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

bool check_simulate(int argc, char *argv[]) {
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--simulate") == 0)
            return true;
    }
    return false;
}

class RPCHandler : virtual public RPCIf {
  private:
    inline static oe_enclave_t *enclave;
    std::unique_ptr<StorageInterface> storage_server;

  public:
    RPCHandler() : storage_server{std::make_unique<redisCli>(HOST_IP)} {}

    static void setEnclaveArgs(int argc, char *argv[]) {
        assert(argc >= 2);

        char *oe_enclave_path = argv[1];
        uint32_t oe_flag = OE_ENCLAVE_FLAG_DEBUG;

        if (check_simulate(argc, argv)) {
#ifdef DEBUG
            spdlog::debug("Running in simulation mode");
#endif

            oe_flag |= OE_ENCLAVE_FLAG_SIMULATE;
        }

        oe_result_t result = oe_create_ortoa_enclave(oe_enclave_path, OE_ENCLAVE_TYPE_SGX, oe_flag, NULL, 0, &enclave);
        if (result != OE_OK) {
            throw OECreationFailed(oe_enclave_path);
        }
    }

    void access(std::string &_return, const Operation &operation) {
        std::string rd_value = storage_server->get(operation.key);

        std::unique_ptr<unsigned char> out(new unsigned char[4096]);
        size_t out_len;

        oe_result_t result = access_data(enclave, operation.op, rd_value.c_str(), rd_value.length(),
                                         operation.value.c_str(), operation.value.length(), out.get(), &out_len);

        if (result == OE_OK) {
            std::string updated_val((const char *)out.get(), out_len);

#ifdef DEBUG
            spdlog::debug("Host | Output of access_data , {0} with len {1}", updated_val, out_len);
#endif

            storage_server->put(operation.key, updated_val);
            _return = updated_val;
        }
    }
};

int main(int argc, char *argv[]) {
    RPCHandler::setEnclaveArgs(argc, argv);

    try {
        auto handler = std::make_shared<RPCHandler>();
        auto processor = std::make_shared<RPCProcessor>(handler);
        auto serverTransport = std::make_shared<TServerSocket>(HOST_PORT);
        auto transportFactory = std::make_shared<TBufferedTransportFactory>();
        auto protocolFactory = std::make_shared<TBinaryProtocolFactory>();

        std::shared_ptr<ThreadFactory> threadFactory = std::shared_ptr<ThreadFactory>(new ThreadFactory());
        std::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(16);
        threadManager->threadFactory(threadFactory);
        threadManager->start();

        std::shared_ptr<apache::thrift::server::TServer> server;
        server.reset(
            new TThreadPoolServer(processor, serverTransport, transportFactory, protocolFactory, threadManager));
        server->serve();
    } catch (OECreationFailed err) {
        spdlog::error("Host | {0}", err.what());
        return 1;
    }
}
