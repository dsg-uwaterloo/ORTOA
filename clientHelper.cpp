#include "clientHelper.h"

#include <algorithm>
#include <fstream>
#include <atomic>

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
std::unordered_map<std::string, std::atomic<bool>> locks;
BS::thread_pool* pool;


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
  for(int i = 0; i < KEY_MAX; i++){
    locks[std::to_string(i)] = ATOMIC_VAR_INIT(true);
  }
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


bool createEntryParallel(int part, char* paddedVal, unsigned char* masterKey, unsigned char* label) {
  int partSize = (VALUE_SIZE / NUM_THREADS) + (VALUE_SIZE % NUM_THREADS != 0);
  int start = part * partSize;
  int limit = std::min((part + 1) * partSize, VALUE_SIZE);

  label += part * partSize * 4 * (1 + crypto_secretbox_KEYBYTES);

  unsigned char* tmpAux = (unsigned char*)malloc(crypto_secretbox_KEYBYTES);

  char c;
  for(int i = start; i < limit; i++) {
    c = paddedVal[i];
    for(int j = 0; j < 4; j++) {
      crypto_kdf_derive_from_key(tmpAux, crypto_secretbox_KEYBYTES, 5*(4*i + j), CONTEXT, masterKey);
      label[0] = (char)((tmpAux[0] & 3) ^ getBits(c, 2*j));
      label += 1;
      crypto_kdf_derive_from_key(label, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 1 + getBits(c, 2*j), CONTEXT, masterKey);
      label += crypto_secretbox_KEYBYTES;
    }
  }
  free(tmpAux);
  return true;
}

Entry constructCreateEntry(std::string& key, std::string& value) {
  Entry entry;
  entry.__set_keyName(key);

  std::string paddedVal = padToLen(value, VALUE_SIZE);
  masterKeys[key] = "";
  masterKeys[key].resize(crypto_kdf_KEYBYTES);
  unsigned char* masterKey = (unsigned char*) &masterKeys[key][0];
  crypto_kdf_keygen(masterKey);


  entry.encryptedLabelsA.resize(VALUE_SIZE*4*(1 + crypto_secretbox_KEYBYTES));
  unsigned char* label = (unsigned char*) &entry.encryptedLabelsA[0];

  std::future<bool> createThreads[NUM_THREADS];

  for(int i = 0; i < NUM_THREADS; i++) {
    createThreads[i] = pool->submit(createEntryParallel, i, &paddedVal[0], masterKey, label);
  }

  for(int i = 0; i < NUM_THREADS; i++) {
    createThreads[i].get();
  }
  return entry;
}

bool getEntryParallel(int part, std::string* key, unsigned char* encryptedLabelA, unsigned char* encryptedLabelB, unsigned char* encryptedLabelC, unsigned char* encryptedLabelD, unsigned char* newMasterKey) {
  int partSize = (VALUE_SIZE / NUM_THREADS) + (VALUE_SIZE % NUM_THREADS != 0);
  int start = part * partSize;
  int limit = std::min((part + 1) * partSize, VALUE_SIZE);

  encryptedLabelA += start * 4 * (CIPHERTEXT_LEN + crypto_secretbox_NONCEBYTES);
  encryptedLabelB += start * 4 * (CIPHERTEXT_LEN + crypto_secretbox_NONCEBYTES);
  encryptedLabelC += start * 4 * (CIPHERTEXT_LEN + crypto_secretbox_NONCEBYTES);
  encryptedLabelD += start * 4 * (CIPHERTEXT_LEN + crypto_secretbox_NONCEBYTES);



  unsigned char* tmpAux = (unsigned char*)malloc(crypto_secretbox_KEYBYTES);

  unsigned char* newLabels[4];
  unsigned char* oldLabels[4];

  newLabels[0] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES + 1);
  newLabels[1] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES + 1);
  newLabels[2] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES + 1);
  newLabels[3] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES + 1);

  oldLabels[0] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  oldLabels[1] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  oldLabels[2] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  oldLabels[3] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);

  for(int i = start; i < limit; i++) {
    for(int j = 0; j < 4; j++) {
      crypto_kdf_derive_from_key(tmpAux, crypto_secretbox_KEYBYTES, 5*(4*i + j), CONTEXT, (unsigned char*) &masterKeys[*key][0]);

      char oldBits = tmpAux[0] & 3;


      crypto_kdf_derive_from_key(tmpAux, crypto_secretbox_KEYBYTES, 5*(4*i + j), CONTEXT, newMasterKey);
      newLabels[0 ^ oldBits][0] = (tmpAux[0] & 3) ^ 0 ^ oldBits;
      newLabels[1 ^ oldBits][0] = (tmpAux[0] & 3) ^ 1 ^ oldBits;
      newLabels[2 ^ oldBits][0] = (tmpAux[0] & 3) ^ 2 ^ oldBits;
      newLabels[3 ^ oldBits][0] = (tmpAux[0] & 3) ^ 3 ^ oldBits;


      crypto_kdf_derive_from_key(newLabels[0] + 1, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 1, CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabels[1] + 1, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 2, CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabels[2] + 1, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 3, CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabels[3] + 1, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 4, CONTEXT, newMasterKey);

      crypto_kdf_derive_from_key(oldLabels[0], crypto_secretbox_KEYBYTES, 5*(4*i + j) + 1, CONTEXT, (unsigned char*) &masterKeys[*key][0]);
      crypto_kdf_derive_from_key(oldLabels[1], crypto_secretbox_KEYBYTES, 5*(4*i + j) + 2, CONTEXT, (unsigned char*) &masterKeys[*key][0]);
      crypto_kdf_derive_from_key(oldLabels[2], crypto_secretbox_KEYBYTES, 5*(4*i + j) + 3, CONTEXT, (unsigned char*) &masterKeys[*key][0]);
      crypto_kdf_derive_from_key(oldLabels[3], crypto_secretbox_KEYBYTES, 5*(4*i + j) + 4, CONTEXT, (unsigned char*) &masterKeys[*key][0]);


      randombytes_buf(encryptedLabelA, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelB, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelC, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelD, crypto_secretbox_NONCEBYTES);

      crypto_secretbox_easy(encryptedLabelA + crypto_secretbox_NONCEBYTES, newLabels[0 ^ oldBits], crypto_secretbox_KEYBYTES + 1, encryptedLabelA, oldLabels[0 ^ oldBits]);
      crypto_secretbox_easy(encryptedLabelB + crypto_secretbox_NONCEBYTES, newLabels[1 ^ oldBits], crypto_secretbox_KEYBYTES + 1, encryptedLabelB, oldLabels[1 ^ oldBits]);
      crypto_secretbox_easy(encryptedLabelC + crypto_secretbox_NONCEBYTES, newLabels[2 ^ oldBits], crypto_secretbox_KEYBYTES + 1, encryptedLabelC, oldLabels[2 ^ oldBits]);
      crypto_secretbox_easy(encryptedLabelD + crypto_secretbox_NONCEBYTES, newLabels[3 ^ oldBits], crypto_secretbox_KEYBYTES + 1, encryptedLabelD, oldLabels[3 ^ oldBits]);

      encryptedLabelA += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelB += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelC += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelD += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
    }
  }
  free(tmpAux);

  for(int i = 0; i < 4; i++){
    free(newLabels[i]);
    free(oldLabels[i]);
  }
  return true;

}

Entry constructGetEntry(std::string& key) {
  Entry entry;
  entry.__set_keyName(key);

  std::string newMasterKeyStr;
  newMasterKeyStr.resize(crypto_kdf_KEYBYTES);
  unsigned char* newMasterKey = (unsigned char*) &newMasterKeyStr[0];
  crypto_kdf_keygen(newMasterKey);



  entry.encryptedLabelsA.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelA = (unsigned char*) &entry.encryptedLabelsA[0];
  entry.encryptedLabelsB.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelB = (unsigned char*) &entry.encryptedLabelsB[0];
  entry.encryptedLabelsC.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelC = (unsigned char*) &entry.encryptedLabelsC[0];
  entry.encryptedLabelsD.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelD = (unsigned char*) &entry.encryptedLabelsD[0];



  std::future<bool> getThreads[NUM_THREADS];

  for(int i = 0; i < NUM_THREADS; i++) {
    getThreads[i] = pool->submit(getEntryParallel, i, &key, encryptedLabelA, encryptedLabelB, encryptedLabelC, encryptedLabelD, newMasterKey);
  }

  for(int i = 0; i < NUM_THREADS; i++) {
    getThreads[i].get();
  }
  
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



  unsigned char* currLabel = (unsigned char*) labels.data() + 1;

  char c;
  for(int i = 0; i < VALUE_SIZE; i++) {
    for(int j = 0; j < 4; j++) {

      crypto_kdf_derive_from_key(label00, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 1, CONTEXT, (unsigned char*)masterKeys[key].data());
      crypto_kdf_derive_from_key(label01, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 2, CONTEXT, (unsigned char*)masterKeys[key].data());
      crypto_kdf_derive_from_key(label10, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 3, CONTEXT, (unsigned char*)masterKeys[key].data());
      crypto_kdf_derive_from_key(label11, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 4, CONTEXT, (unsigned char*)masterKeys[key].data());

      c = modifyBits(c, 2*j, !memcmp(currLabel, label00, crypto_secretbox_KEYBYTES) ? 0 : !memcmp(currLabel, label01, crypto_secretbox_KEYBYTES) ? 1 :
                            !memcmp(currLabel, label10, crypto_secretbox_KEYBYTES) ? 2: 3);

      currLabel += crypto_secretbox_KEYBYTES + 1;
    }
    result[i] = c;
  }

  free(label00);
  free(label01);
  free(label10);
  free(label11);

  return result.substr(0, valueSizes[key]);
}


bool putEntryParallel(int part, std::string* key, std::string* paddedVal, unsigned char* encryptedLabelA, unsigned char* encryptedLabelB, unsigned char* encryptedLabelC, unsigned char* encryptedLabelD, unsigned char* newMasterKey) {
  int partSize = (VALUE_SIZE / NUM_THREADS) + (VALUE_SIZE % NUM_THREADS != 0);
  int start = part * partSize;
  int limit = std::min((part + 1) * partSize, VALUE_SIZE);

  unsigned char* newLabels[4];
  unsigned char* oldLabels[4];

  encryptedLabelA += start * 4 * (CIPHERTEXT_LEN + crypto_secretbox_NONCEBYTES);
  encryptedLabelB += start * 4 * (CIPHERTEXT_LEN + crypto_secretbox_NONCEBYTES);
  encryptedLabelC += start * 4 * (CIPHERTEXT_LEN + crypto_secretbox_NONCEBYTES);
  encryptedLabelD += start * 4 * (CIPHERTEXT_LEN + crypto_secretbox_NONCEBYTES);

  newLabels[0] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES + 1);
  newLabels[1] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES + 1);
  newLabels[2] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES + 1);
  newLabels[3] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES + 1);

  oldLabels[0] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  oldLabels[1] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  oldLabels[2] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);
  oldLabels[3] = (unsigned char*) malloc(crypto_secretbox_KEYBYTES);


  char c;
  int newBits;

  unsigned char* tmpAux = (unsigned char*)malloc(crypto_secretbox_KEYBYTES);

  for(int i = start; i < limit; i++) {
    c = paddedVal->at(i);
    for(int j = 0; j < 4; j++) {

      crypto_kdf_derive_from_key(tmpAux, crypto_secretbox_KEYBYTES, 5*(4*i + j), CONTEXT, (unsigned char*) &masterKeys[*key][0]);
      char oldBits = tmpAux[0] & 3;

      crypto_kdf_derive_from_key(tmpAux, crypto_secretbox_KEYBYTES, 5*(4*i + j), CONTEXT, newMasterKey);

      newLabels[0 ^ oldBits][0] = (tmpAux[0] & 3) ^ oldBits ^ 0;
      newLabels[1 ^ oldBits][0] = (tmpAux[0] & 3) ^ oldBits ^ 1;
      newLabels[2 ^ oldBits][0] = (tmpAux[0] & 3) ^ oldBits ^ 2;
      newLabels[3 ^ oldBits][0] = (tmpAux[0] & 3) ^ oldBits ^ 3;

      crypto_kdf_derive_from_key(newLabels[0] + 1, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 1, CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabels[1] + 1, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 2, CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabels[2] + 1, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 3, CONTEXT, newMasterKey);
      crypto_kdf_derive_from_key(newLabels[3] + 1, crypto_secretbox_KEYBYTES, 5*(4*i + j) + 4, CONTEXT, newMasterKey);

      crypto_kdf_derive_from_key(oldLabels[0], crypto_secretbox_KEYBYTES, 5*(4*i + j) + 1, CONTEXT, (unsigned char*) &masterKeys[*key][0]);
      crypto_kdf_derive_from_key(oldLabels[1], crypto_secretbox_KEYBYTES, 5*(4*i + j) + 2, CONTEXT, (unsigned char*) &masterKeys[*key][0]);
      crypto_kdf_derive_from_key(oldLabels[2], crypto_secretbox_KEYBYTES, 5*(4*i + j) + 3, CONTEXT, (unsigned char*) &masterKeys[*key][0]);
      crypto_kdf_derive_from_key(oldLabels[3], crypto_secretbox_KEYBYTES, 5*(4*i + j) + 4, CONTEXT, (unsigned char*) &masterKeys[*key][0]);

      randombytes_buf(encryptedLabelA, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelB, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelC, crypto_secretbox_NONCEBYTES);
      randombytes_buf(encryptedLabelD, crypto_secretbox_NONCEBYTES);

      newBits = getBits(c, 2*j);

      unsigned char* newLabel = 0;
      newLabel = newLabels[newBits];


      crypto_secretbox_easy(encryptedLabelA + crypto_secretbox_NONCEBYTES, newLabel, crypto_secretbox_KEYBYTES + 1, encryptedLabelA, oldLabels[0 ^ oldBits]);
      crypto_secretbox_easy(encryptedLabelB + crypto_secretbox_NONCEBYTES, newLabel, crypto_secretbox_KEYBYTES + 1, encryptedLabelB, oldLabels[1 ^ oldBits]);
      crypto_secretbox_easy(encryptedLabelC + crypto_secretbox_NONCEBYTES, newLabel, crypto_secretbox_KEYBYTES + 1, encryptedLabelC, oldLabels[2 ^ oldBits]);
      crypto_secretbox_easy(encryptedLabelD + crypto_secretbox_NONCEBYTES, newLabel, crypto_secretbox_KEYBYTES + 1, encryptedLabelD, oldLabels[3 ^ oldBits]);

      encryptedLabelA += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelB += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelC += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
      encryptedLabelD += crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN;
    }
  }
  free(tmpAux);
  for(int i = 0; i < 4; i++){
    free(newLabels[i]);
    free(oldLabels[i]);
  }
  return true;
}

Entry constructPutEntry(std::string& key, std::string& value) {
  Entry entry;
  entry.__set_keyName(key);

  std::string paddedVal = padToLen(value, VALUE_SIZE);

  std::string newMasterKeyStr;
  newMasterKeyStr.resize(crypto_kdf_KEYBYTES);
  unsigned char* newMasterKey = (unsigned char*) &newMasterKeyStr[0];
  crypto_kdf_keygen(newMasterKey);

  

  entry.encryptedLabelsA.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelA = (unsigned char*) &entry.encryptedLabelsA[0];
  entry.encryptedLabelsB.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelB = (unsigned char*) &entry.encryptedLabelsB[0];
  entry.encryptedLabelsC.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelC = (unsigned char*) &entry.encryptedLabelsC[0];
  entry.encryptedLabelsD.resize(VALUE_SIZE * 4 * (crypto_secretbox_NONCEBYTES + CIPHERTEXT_LEN));
  unsigned char* encryptedLabelD = (unsigned char*) &entry.encryptedLabelsD[0];


  std::future<bool> putThreads[NUM_THREADS];

  for(int i = 0; i < NUM_THREADS; i++) {
    putThreads[i] = pool->submit(putEntryParallel, i, &key, &paddedVal, encryptedLabelA, encryptedLabelB, encryptedLabelC, encryptedLabelD, newMasterKey);
  }

  for(int i = 0; i < NUM_THREADS; i++) {
    putThreads[i].get();
  }
  

  masterKeys[key] = newMasterKeyStr;

  return entry;
}

