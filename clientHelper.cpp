#include "clientHelper.h"

#include <thread>
#include <algorithm>
#include <fstream>

#include <boost/filesystem.hpp>

#include <boost/archive/tmpdir.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/unordered_map.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#define NUM_THREADS 8

std::set<std::string> keySet;
std::unordered_map<std::string, int> valueSizes;
std::unordered_map<std::string, std::string> masterKeys;

unsigned char randomBytes[VALUE_SIZE];

inline std::string padToLen(std::string& value, int len) {
  return value + std::string(len - value.length(), ' ');
}

inline int getBit(unsigned char c, int bitNum) {
  return (c >> bitNum) & 1;
}

inline int getBits(unsigned char c, int bitNumPair) {
  return (c >> (bitNumPair)) & 3;
}

inline unsigned char modifyBit(unsigned char c, int bitNum, int b)
{
  return ((c & ~(1 << bitNum)) | (b << bitNum));
}

inline unsigned char modifyBits(unsigned char c, int bitNum, int b)
{
  unsigned char p = modifyBit(c, bitNum, b & 1);
  p = modifyBit(p, bitNum + 1, (b >> 1) & 1);
  return p;
}

void OpScureSetup(std::string fileName) {
	if(boost::filesystem::exists(fileName)){
		std::ifstream ifs(fileName);
		boost::archive::text_iarchive ia(ifs);
		ia >> keySet;
		ia >> valueSizes;
		ia >> masterKeys;
		ifs.close();
	}
}


void OpScureCleanup(std::string fileName) {
	std::ofstream ofs(fileName);
	boost::archive::text_oarchive oa(ofs);
	oa << keySet;
	oa << valueSizes;
	oa << masterKeys;
	ofs.close();
}


void createEntryParallel(int part, char* paddedVal, unsigned char* masterKey, unsigned char* label) {
  int partSize = (VALUE_SIZE / NUM_THREADS) + (VALUE_SIZE % NUM_THREADS != 0);
  int start = part * partSize;
  int limit = std::min((part + 1) * partSize, VALUE_SIZE);

  label += part * partSize * 4 * crypto_secretbox_KEYBYTES;

  char c;
  for(int i = start; i < limit; i++) {
    c = paddedVal[i];
    for(int j = 0; j < 4; j++) {
      crypto_kdf_derive_from_key(label, crypto_secretbox_KEYBYTES, 4*(4*i + j) + getBits(c, 2*j), CONTEXT, masterKey);
      label += crypto_secretbox_KEYBYTES;
    }
  }
}

Entry constructCreateEntry(std::string& key, std::string& value) {
  Entry entry;
  entry.__set_keyName(key);

  std::string paddedVal = padToLen(value, VALUE_SIZE);
  masterKeys[key] = "";
  masterKeys[key].resize(crypto_kdf_KEYBYTES);
  unsigned char* masterKey = (unsigned char*) &masterKeys[key][0];
  crypto_kdf_keygen(masterKey);

  entry.encryptedLabelsA.resize(VALUE_SIZE*4*crypto_secretbox_KEYBYTES);
  unsigned char* label = (unsigned char*) &entry.encryptedLabelsA[0];

  std::thread createThreads[NUM_THREADS];

  for(int i = 0; i < NUM_THREADS; i++) {
    createThreads[i] = std::thread(createEntryParallel, i, &paddedVal[0], masterKey, label);
  }

  for(int i = 0; i < NUM_THREADS; i++) {
    createThreads[i].join();
  }
  return entry;
}

void getEntryParallel(int part, unsigned char* randomBytes) {
  int partSize = (VALUE_SIZE / NUM_THREADS) + (VALUE_SIZE % NUM_THREADS != 0);
  int start = part * partSize;
  int limit = std::min((part + 1) * partSize, VALUE_SIZE);
}

Entry constructGetEntry(std::string& key) {
  Entry entry;
  entry.__set_keyName(key);

  std::string newMasterKeyStr;
  newMasterKeyStr.resize(crypto_kdf_KEYBYTES);
  unsigned char* newMasterKey = (unsigned char*) &newMasterKeyStr[0];
  crypto_kdf_keygen(newMasterKey);

  unsigned char* newLabel00 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* newLabel01 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* newLabel10 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* newLabel11 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);

  unsigned char* oldLabel00 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* oldLabel01 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* oldLabel10 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* oldLabel11 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);

  entry.encryptedLabelsA.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelA = (unsigned char*) &entry.encryptedLabelsA[0];
  entry.encryptedLabelsB.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelB = (unsigned char*) &entry.encryptedLabelsB[0];
  entry.encryptedLabelsC.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelC = (unsigned char*) &entry.encryptedLabelsC[0];
  entry.encryptedLabelsD.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelD = (unsigned char*) &entry.encryptedLabelsD[0];

  randombytes_buf(randomBytes, VALUE_SIZE);

  char randomBits;

  for(int i = 0; i < VALUE_SIZE; i++) {
    for(int j = 0; j < 4; j++) {

      crypto_kdf_derive_from_key(newLabel00, crypto_secretbox_KEYBYTES, 4*(4*i + j), CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabel01, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 1, CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabel10, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 2, CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabel11, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 3, CONTEXT, newMasterKey);

      crypto_kdf_derive_from_key(oldLabel00, crypto_secretbox_KEYBYTES, 4*(4*i + j), CONTEXT, (unsigned char*) &masterKeys[key][0]);
      crypto_kdf_derive_from_key(oldLabel01, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 1, CONTEXT, (unsigned char*) &masterKeys[key][0]);
      crypto_kdf_derive_from_key(oldLabel10, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 2, CONTEXT, (unsigned char*) &masterKeys[key][0]);
      crypto_kdf_derive_from_key(oldLabel11, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 3, CONTEXT, (unsigned char*) &masterKeys[key][0]);


      randombytes_buf(encryptedLabelA, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelB, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelC, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelD, crypto_secretbox_NONCEBYTES);

      randomBits = getBits(randomBytes[i], 2*j);

      crypto_secretbox_easy(encryptedLabelA + crypto_secretbox_NONCEBYTES, randomBits & 1 ? newLabel00 : newLabel01, crypto_secretbox_KEYBYTES, encryptedLabelA, randomBits & 1 ? oldLabel00 : oldLabel01);
      crypto_secretbox_easy(encryptedLabelB + crypto_secretbox_NONCEBYTES, randomBits & 1 ? newLabel01 : newLabel00, crypto_secretbox_KEYBYTES, encryptedLabelB, randomBits & 1 ? oldLabel01 : oldLabel00);
      crypto_secretbox_easy(encryptedLabelC + crypto_secretbox_NONCEBYTES, randomBits & 2 ? newLabel10 : newLabel11, crypto_secretbox_KEYBYTES, encryptedLabelC, randomBits & 2 ? oldLabel10 : oldLabel11);
      crypto_secretbox_easy(encryptedLabelD + crypto_secretbox_NONCEBYTES, randomBits & 2 ? newLabel11 : newLabel10, crypto_secretbox_KEYBYTES, encryptedLabelD, randomBits & 2 ? oldLabel11 : oldLabel10);

      encryptedLabelA += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelB += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelC += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelD += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
    }
  }

  free(newLabel00);
  free(newLabel01);
  free(newLabel10);
  free(newLabel11);

  free(oldLabel00);
  free(oldLabel01);
  free(oldLabel10);
  free(oldLabel11);

  masterKeys[key] = newMasterKeyStr;

  return entry;
}

std::string readValueFromLabels(std::string key, std::string labels) {
  std::string result;
  result.resize(VALUE_SIZE);

  unsigned char* label00 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* label01 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* label10 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* label11 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);



  unsigned char* currLabel = (unsigned char*) labels.data();

  char c;
  for(int i = 0; i < VALUE_SIZE; i++) {
    for(int j = 0; j < 4; j++) {

      crypto_kdf_derive_from_key(label00, crypto_secretbox_KEYBYTES, 4*(4*i + j), CONTEXT, (unsigned char*)masterKeys[key].data());
      crypto_kdf_derive_from_key(label01, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 1, CONTEXT, (unsigned char*)masterKeys[key].data());
      crypto_kdf_derive_from_key(label10, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 2, CONTEXT, (unsigned char*)masterKeys[key].data());
      crypto_kdf_derive_from_key(label11, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 3, CONTEXT, (unsigned char*)masterKeys[key].data());

      c = modifyBits(c, 2*j, !memcmp(currLabel, label00, crypto_secretbox_KEYBYTES) ? 0 : !memcmp(currLabel, label01, crypto_secretbox_KEYBYTES) ? 1 :
                            !memcmp(currLabel, label10, crypto_secretbox_KEYBYTES) ? 2: 3);

      currLabel += crypto_secretbox_KEYBYTES;
    }
    result[i] = c;
  }

  free(label00);
  free(label01);
  free(label10);
  free(label11);

  return result.substr(0, valueSizes[key]);
}

Entry constructPutEntry(std::string& key, std::string& value) {
  Entry entry;
  entry.__set_keyName(key);

  std::string paddedVal = padToLen(value, VALUE_SIZE);

  std::string newMasterKeyStr;
  newMasterKeyStr.resize(crypto_kdf_KEYBYTES);
  unsigned char* newMasterKey = (unsigned char*) &newMasterKeyStr[0];
  crypto_kdf_keygen(newMasterKey);

  unsigned char* newLabel00 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* newLabel01 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* newLabel10 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* newLabel11 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);


  unsigned char* oldLabel00 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* oldLabel01 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* oldLabel10 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  unsigned char* oldLabel11 = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);

  entry.encryptedLabelsA.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelA = (unsigned char*) &entry.encryptedLabelsA[0];
  entry.encryptedLabelsB.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelB = (unsigned char*) &entry.encryptedLabelsB[0];
  entry.encryptedLabelsC.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelC = (unsigned char*) &entry.encryptedLabelsC[0];
  entry.encryptedLabelsD.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelD = (unsigned char*) &entry.encryptedLabelsD[0];


  randombytes_buf(randomBytes, VALUE_SIZE);

  char c;
  int newBits, randomBit;

  for(int i = 0; i < VALUE_SIZE; i++) {
    c = paddedVal.at(i);
    for(int j = 0; j < 4; j++) {

      crypto_kdf_derive_from_key(newLabel00, crypto_secretbox_KEYBYTES, 4*(4*i + j), CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabel01, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 1, CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabel10, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 2, CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabel11, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 3, CONTEXT, newMasterKey);

      crypto_kdf_derive_from_key(oldLabel00, crypto_secretbox_KEYBYTES, 4*(4*i + j), CONTEXT, (unsigned char*) &masterKeys[key][0]);
      crypto_kdf_derive_from_key(oldLabel01, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 1, CONTEXT, (unsigned char*) &masterKeys[key][0]);
      crypto_kdf_derive_from_key(oldLabel10, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 2, CONTEXT, (unsigned char*) &masterKeys[key][0]);
      crypto_kdf_derive_from_key(oldLabel11, crypto_secretbox_KEYBYTES, 4*(4*i + j) + 3, CONTEXT, (unsigned char*) &masterKeys[key][0]);

      randombytes_buf(encryptedLabelA, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelB, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelC, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelD, crypto_secretbox_NONCEBYTES);

      newBits = getBits(c, 2*j);
      randomBit = getBits(randomBytes[i], 2*j);

      unsigned char* newLabel = 0;
      if(newBits == 0){
        newLabel = newLabel00;
      }
      else if(newBits == 1){
        newLabel = newLabel01;
      }
      else if(newBits == 2){
        newLabel = newLabel10;
      }
      else{
        newLabel = newLabel11;
      }


      crypto_secretbox_easy(encryptedLabelA + crypto_secretbox_NONCEBYTES, newLabel, crypto_secretbox_KEYBYTES, encryptedLabelA, randomBit & 1 ? oldLabel00 : oldLabel01);
      crypto_secretbox_easy(encryptedLabelB + crypto_secretbox_NONCEBYTES, newLabel, crypto_secretbox_KEYBYTES, encryptedLabelB, randomBit & 1 ? oldLabel01 : oldLabel00);
      crypto_secretbox_easy(encryptedLabelC + crypto_secretbox_NONCEBYTES, newLabel, crypto_secretbox_KEYBYTES, encryptedLabelC, randomBit & 2 ? oldLabel10 : oldLabel11);
      crypto_secretbox_easy(encryptedLabelD + crypto_secretbox_NONCEBYTES, newLabel, crypto_secretbox_KEYBYTES, encryptedLabelD, randomBit & 2 ? oldLabel11 : oldLabel10);

      encryptedLabelA += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelB += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelC += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelD += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
    }
  }

  free(newLabel00);
  free(newLabel01);
  free(newLabel10);
  free(newLabel11);

  free(oldLabel00);
  free(oldLabel01);
  free(oldLabel10);
  free(oldLabel11);

  masterKeys[key] = newMasterKeyStr;

  return entry;
}

