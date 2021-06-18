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
#include <utility>
#include <iostream>
#include <unordered_map>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <sodium.h>

#include "gen-cpp/KV_RPC.h"

#define VALUE_SIZE 1024

#define CIPHERTEXT_LEN (crypto_secretbox_MACBYTES + crypto_secretbox_KEYBYTES)

#define CONTEXT "OpScureK"

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

std::set<std::string> keySet;
std::unordered_map<std::string, int> valueSizes;
std::unordered_map<std::string, unsigned char*> masterKeys;

unsigned char randomBytes[VALUE_SIZE];

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

inline std::string padToLen(std::string& value, int len) {
  return value + std::string(len - value.length(), ' ');
}

inline int getBit(unsigned char c, int bitNum) {
  return (c >> bitNum) & 1;
}

inline unsigned char modifyBit(unsigned char c, int bitNum, int b)
{
    return ((c & ~(1 << bitNum)) | (b << bitNum));
}

Entry constructCreateEntry(std::string& key, std::string& value) {
  Entry entry;
  std::string paddedVal = padToLen(value, VALUE_SIZE);
  masterKeys[key] = (unsigned char*) malloc(crypto_kdf_KEYBYTES);
  crypto_kdf_keygen(masterKeys[key]);

  entry.encryptedLabelsA.resize(VALUE_SIZE*8*crypto_secretbox_KEYBYTES);
  unsigned char* label = (unsigned char*) entry.encryptedLabelsA.data();

  char c;
  for(int i = 0; i < VALUE_SIZE; i++) {
    c = paddedVal.at(i);
    for(int j = 0; j < 8; j++) {
      crypto_kdf_derive_from_key(label, crypto_secretbox_KEYBYTES, 2*(8*i + j) + getBit(c, j), CONTEXT, masterKeys[key]);
      label += crypto_secretbox_KEYBYTES;
    }
  }

  return entry;
}

Entry constructGetEntry(std::string& key) {
  Entry entry;

  unsigned char* newMasterKey = (unsigned char*) malloc(crypto_kdf_KEYBYTES);
  crypto_kdf_keygen(newMasterKey);

  unsigned char* newLabel0 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* newLabel1 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);

  unsigned char* oldLabel0 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* oldLabel1 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);

  entry.encryptedLabelsA.resize(VALUE_SIZE * 8 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelA = (unsigned char*) entry.encryptedLabelsA.data();
  entry.encryptedLabelsB.resize(VALUE_SIZE * 8 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelB = (unsigned char*) entry.encryptedLabelsB.data();

  randombytes_buf(randomBytes, VALUE_SIZE);

  char randomBit;

  for(int i = 0; i < VALUE_SIZE; i++) {
    for(int j = 0; j < 8; j++) {

      crypto_kdf_derive_from_key(newLabel0, crypto_secretbox_KEYBYTES, 2*(8*i + j), CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabel1, crypto_secretbox_KEYBYTES, 2*(8*i + j) + 1, CONTEXT, newMasterKey);

      crypto_kdf_derive_from_key(oldLabel0, crypto_secretbox_KEYBYTES, 2*(8*i + j), CONTEXT, masterKeys[key]);
      crypto_kdf_derive_from_key(oldLabel1, crypto_secretbox_KEYBYTES, 2*(8*i + j) + 1, CONTEXT, masterKeys[key]);

      randombytes_buf(encryptedLabelA, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelB, crypto_secretbox_NONCEBYTES);

      randomBit = getBit(randomBytes[i], j);

      crypto_secretbox_easy(encryptedLabelA + crypto_secretbox_NONCEBYTES, randomBit ? newLabel0 : newLabel1, crypto_secretbox_KEYBYTES, encryptedLabelA, randomBit ? oldLabel0 : oldLabel1);
      crypto_secretbox_easy(encryptedLabelB + crypto_secretbox_NONCEBYTES, randomBit ? newLabel1 : newLabel0, crypto_secretbox_KEYBYTES, encryptedLabelB, randomBit ? oldLabel1 : oldLabel0);

      encryptedLabelA += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelB += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
    }
  }

  free(newLabel0);
  free(newLabel1);

  free(oldLabel0);
  free(oldLabel1);

  free(masterKeys[key]);
  masterKeys[key] = newMasterKey;

  return entry;
}

std::string readValueFromLabels(std::string key, std::string labels) {
  std::string result;
  result.resize(VALUE_SIZE);

  unsigned char* label0 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);

  unsigned char* currLabel = (unsigned char*) labels.data();

  char c;
  for(int i = 0; i < VALUE_SIZE; i++) {
    for(int j = 0; j < 8; j++) {

      crypto_kdf_derive_from_key(label0, crypto_secretbox_KEYBYTES, 2*(8*i + j), CONTEXT, masterKeys[key]);

      c = modifyBit(c, j, memcmp(currLabel, label0, crypto_secretbox_KEYBYTES) ? 1 : 0);

      currLabel += crypto_secretbox_KEYBYTES;
    }
    result[i] = c;
  }

  free(label0);

  return result.substr(0, valueSizes[key]);
}

Entry constructPutEntry(std::string& key, std::string& value) {
  Entry entry;

  std::string paddedVal = padToLen(value, VALUE_SIZE);

  unsigned char* newMasterKey = (unsigned char*) malloc(crypto_kdf_KEYBYTES);
  crypto_kdf_keygen(newMasterKey);

  unsigned char* newLabel0 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* newLabel1 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);

  unsigned char* oldLabel0 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* oldLabel1 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);

  entry.encryptedLabelsA.resize(VALUE_SIZE * 8 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelA = (unsigned char*) entry.encryptedLabelsA.data();
  entry.encryptedLabelsB.resize(VALUE_SIZE * 8 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelB = (unsigned char*) entry.encryptedLabelsB.data();

  randombytes_buf(randomBytes, VALUE_SIZE);

  char c, newBit, randomBit;

  for(int i = 0; i < VALUE_SIZE; i++) {
    c = paddedVal.at(i);
    for(int j = 0; j < 8; j++) {

      crypto_kdf_derive_from_key(newLabel0, crypto_secretbox_KEYBYTES, 2*(8*i + j), CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabel1, crypto_secretbox_KEYBYTES, 2*(8*i + j) + 1, CONTEXT, newMasterKey);

      crypto_kdf_derive_from_key(oldLabel0, crypto_secretbox_KEYBYTES, 2*(8*i + j), CONTEXT, masterKeys[key]);
      crypto_kdf_derive_from_key(oldLabel1, crypto_secretbox_KEYBYTES, 2*(8*i + j) + 1, CONTEXT, masterKeys[key]);

      randombytes_buf(encryptedLabelA, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelB, crypto_secretbox_NONCEBYTES);

      newBit = getBit(c, j);
      randomBit = getBit(randomBytes[i], j);

      crypto_secretbox_easy(encryptedLabelA + crypto_secretbox_NONCEBYTES, newBit ? newLabel1 : newLabel0, crypto_secretbox_KEYBYTES, encryptedLabelA, randomBit ? oldLabel0 : oldLabel1);
      crypto_secretbox_easy(encryptedLabelB + crypto_secretbox_NONCEBYTES, newBit ? newLabel1 : newLabel0, crypto_secretbox_KEYBYTES, encryptedLabelB, randomBit ? oldLabel1 : oldLabel0);

      encryptedLabelA += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelB += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
    }
  }

  free(newLabel0);
  free(newLabel1);

  free(oldLabel0);
  free(oldLabel1);

  free(masterKeys[key]);
  masterKeys[key] = newMasterKey;

  return entry;
}

int main() {
  std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
  std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  KV_RPCClient client(protocol);

  Operation op;

  try {
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
