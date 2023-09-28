#include "../crypto/encryption_engine.h"
#include "../host/redis.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
	encryption_engine encryption_engine_;
	redisCli rd;
	unsigned char cipher_text[4096];
	auto encLen = encryption_engine_.encryptNonDeterministic("30", cipher_text);
	string val((const char *)cipher_text, encLen);
	cout << "Encrypted value is: " << val << endl;
	rd.put("1", val);
	cout << "Redis get encrypted for key 1: " << rd.get("1") << endl;
    cout << "Decrypted value for key 1: " << encryption_engine_.decryptNonDeterministic(rd.get("1")) << endl;
    
    encLen = encryption_engine_.encryptNonDeterministic("20", cipher_text);
	string val1((const char *)cipher_text, encLen);
	rd.put("2", val1);
	cout << "Redis get encrypted for key 2: " << rd.get("2") << endl;
    cout << "Decrypted value for key 1: " << encryption_engine_.decryptNonDeterministic(rd.get("2")) << endl;
	return 0;
}