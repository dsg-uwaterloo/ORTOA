#include "clientHelper.h"

std::set<std::string> keySet;
std::unordered_map<std::string, int> valueSizes;
std::unordered_map<std::string, unsigned char*> masterKeys;

unsigned char randomBytes[VALUE_SIZE];

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

