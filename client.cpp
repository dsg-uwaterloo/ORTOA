/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <set>
#include <string>
#include <iostream>
#include <unordered_map>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/KV_RPC.h"
#include "clientHelper.h"

#define DATA_FILE "/dsl/OpScure/OpScure.data"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

enum OperationType {
  GET,
  PUT
};

struct Operation {
  OperationType type;
  std::string key;
  std::string value;
};

void signal_callback_handler(int signum) {
   OpScureCleanup(DATA_FILE);
   exit(signum);
}

Operation parseOperation() {
  Operation op;
  std::string tmp;
  std::cin >> tmp;
  op.type = (tmp == "PUT") ? PUT : GET;
  std::cin >> op.key;
  if(op.type == PUT)
    std::cin >> op.value;
  return op;
}

int main() {

  signal(SIGINT, signal_callback_handler);

  std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
  std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  KV_RPCClient client(protocol);

  Operation op;

  try {
    OpScureSetup(DATA_FILE);
    transport->open();

    while(1) {
      std::cerr << "> ";
      op = parseOperation();

      if(!keySet.count(op.key)) {
        if(op.type == GET) {
          std::cerr << "No such key exists" << std::endl;
        } else {
          Entry createEntry = constructCreateEntry(op.key, op.value);
          valueSizes[op.key] = op.value.length();
          keySet.insert(op.key);
          client.create(createEntry);
        }
      } else {
        if(op.type == GET) {
          Entry getEntry = constructGetEntry(op.key);
          std::string labels;
          client.access(labels, getEntry);
          std::string value = readValueFromLabels(op.key, labels);
          std::cerr << value << std::endl;
        } else {
          Entry putEntry = constructPutEntry(op.key, op.value);
          valueSizes[op.key] = op.value.length();
          std::string labels;
          client.access(labels, putEntry);
        }
      }

      //std::cerr << (op.type ? "PUT" : "GET") << " " << op.key << " " << op.value << std::endl;
    }    

    transport->close();
  } catch (TException& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }
}
