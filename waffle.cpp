#include "waffle.h"

#include "gen-cpp/KV_RPC.h"
#include "gen-cpp/KV_RPC_types.h"
// #include "gen-cpp/KV_RPC_constants.h"

#include "clientHelper.h"

#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#define DATA_FILE "/dsl/OpScure/OpScure.data"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

std::shared_ptr<TTransport> mySocket;
std::shared_ptr<TTransport> myTransport;
std::shared_ptr<TProtocol> myProtocol;
std::shared_ptr<KV_RPCClient> client;

JNIEXPORT void JNICALL Java_site_ycsb_db_WaffleClient_nativeInit(JNIEnv *, jobject, jint maxDeltaRead, jint maxDeltaWrite, jint cacheSize)
{
	printf("----------------\nInitializing OpScure\n----------------\n");
	fflush(stdout);

	mySocket = std::shared_ptr<TTransport>(new TSocket("localhost", 9090));
	myTransport = std::shared_ptr<TTransport>((new TBufferedTransport(mySocket)));
	myProtocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(myTransport));
	client = std::shared_ptr<KV_RPCClient>(new KV_RPCClient(myProtocol));

	OpScureSetup(DATA_FILE);

	myTransport->open();
	printf("Finished nativeInit\n");

}

JNIEXPORT void JNICALL Java_site_ycsb_db_WaffleClient_nativeCleanup(JNIEnv *, jobject)
{
	printf("----------------\nCleaning up OpScure\n----------------\n");
	fflush(stdout);

	myTransport->close();
	OpScureCleanup(DATA_FILE);
	printf("Finished nativeCleanup\n");


}

JNIEXPORT jint JNICALL Java_site_ycsb_db_WaffleClient_nativeCreate(JNIEnv * env, jobject, jstring jkey, jbyteArray jvalue)
{
	printf("Started nativeCreate\n");

	uint32_t size = env->GetArrayLength(jvalue);
	jbyte* valuePtr = env->GetByteArrayElements(jvalue, 0);

	std::string value((char*)valuePtr, size);
	env->ReleaseByteArrayElements(jvalue, valuePtr, 0);

	jboolean isCopy = false;
	uint8_t keySize = env->GetStringLength(jkey);
	const char* keyChars = env->GetStringUTFChars(jkey, &isCopy);
	std::string key(keyChars, keySize);
	env->ReleaseStringUTFChars(jkey, keyChars);

	Entry createEntry = constructCreateEntry(key, value);
	valueSizes[key] = value.length();
	keySet.insert(key);
	client->create(createEntry);
	printf("Finished nativeCreate\n");


	return 1;
}

JNIEXPORT jbyteArray JNICALL Java_site_ycsb_db_WaffleClient_nativeRead(JNIEnv * env, jobject, jstring jkey)
{
	printf("started nativeRead\n");

	jboolean isCopy;
	uint8_t keySize = env->GetStringLength(jkey);
	const char* keyChars = env->GetStringUTFChars(jkey, &isCopy);
	std::string key(keyChars, keySize);
	env->ReleaseStringUTFChars(jkey, keyChars);
	printf("Here read 1\n");
	printf("%lu\n", keySet.count(key));
	std::cout << key << std::endl;
	if(!keySet.count(key)){
		std::cerr << "No such key exists" << std::endl;
		jbyteArray jData = (env)->NewByteArray(8);
		return jData;
	}
	Entry getEntry = constructGetEntry(key);
	printf("Entry construced\n");
	std::string labels;
	client->access(labels, getEntry);
	std::string value = readValueFromLabels(key, labels);
	printf("Here read 2\n");
	jbyteArray jData = (env)->NewByteArray(value.length());
	env->SetByteArrayRegion(jData, 0, value.length(), (jbyte*) &value[0]);

	printf("Finished nativeRead\n");

	return jData;
}

JNIEXPORT jint JNICALL Java_site_ycsb_db_WaffleClient_nativeUpdate(JNIEnv * env, jobject, jstring jkey, jbyteArray jvalue)
{
	printf("started nativeUpdate\n");

	uint32_t size = env->GetArrayLength(jvalue);
	jbyte* valuePtr = env->GetByteArrayElements(jvalue, 0);

	std::string value((char*)valuePtr, size);


	env->ReleaseByteArrayElements(jvalue, valuePtr, 0);

	jboolean isCopy = false;
	uint8_t keySize = env->GetStringLength(jkey);
	const char* keyChars = env->GetStringUTFChars(jkey, &isCopy);
	std::string key(keyChars, keySize);
	env->ReleaseStringUTFChars(jkey, keyChars);
	printf("%lu\n", keySet.count(key));
	std::cout << key << std::endl;
	if(!keySet.count(key)){
		Entry createEntry = constructCreateEntry(key, value);
		valueSizes[key] = value.length();
		keySet.insert(key);
		client->create(createEntry);
	}
	else{
		Entry putEntry = constructPutEntry(key, value);
		valueSizes[key] = value.length();
		std::string labels;
		client->access(labels, putEntry);
	}
	printf("Finished nativeUpdate\n");

	return 1;
}