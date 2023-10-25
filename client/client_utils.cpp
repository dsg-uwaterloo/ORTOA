#include "client_utils.h"

std::mutex fileMutex;

void parseArgs(int argc, char *argv[], std::ifstream &seed, bool &init_db, int &num_clients, float &p_get) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    
    // Check if current argument is path to the seed data for init DB
    if (arg == "--seed" && i + 1 < argc) {
      std::string seed_name = argv[i + 1];
      seed.open(seed_name);

		  if (!seed.is_open()) {
    	  throw std::invalid_argument("Invalid path to seed data");
      }
      i++; // Skip the next argument
		}

    // Check if current argument is number of clients for multithreading
    else if (arg == "--nthreads" && i + 1 < argc) {
      num_clients = std::stoi(argv[i + 1]);
      i++;
    }

    // Check if current argument is probability of GET operation
    else if (arg == "--pget" && i + 1 < argc) {
      p_get = std::stoi(argv[i + 1]);
      i++;
    }
    
    // Check if client is to initialize database
    else if (arg == "--initdb") {
      init_db = true;
    }
  }
}

Operation genRandOperation(int p_get) {
	float r = (float) rand() / RAND_MAX;
	int key = rand() % KEY_MAX;
  
	Operation op;
	op.__set_op(r <= p_get ? OpType::GET : OpType::PUT);
  op.__set_key(std::to_string(key));

	std::string value;

  // If operation is GET, then set value to random bytes
  // If operation is PUT, then set value to random int value
	if (op.op == OpType::GET) {
		char rand_val[VALUE_SIZE];
		randombytes_buf(rand_val, VALUE_SIZE);
		value = std::string(rand_val);
	} else {
		int put_val = rand() % VAL_MAX;
		value = std::to_string(put_val);
	}
	op.__set_value(clientEncrypt(value));

	return op;
}

Operation getSeedOperation(std::string& line) {
  std::istringstream ss(line);
  std::string operation, key, value;

  std::getline(ss, operation, ',');
  std::getline(ss, key, ',');
  std::getline(ss, value, ',');

  Operation op;

  op.__set_op((operation == "GET") ? OpType::GET : OpType::PUT);
  op.__set_key(key);

  // If operation is GET, then update value to random bytes
  if (op.op == OpType::GET) {
    char rand_val[VALUE_SIZE];
    randombytes_buf(rand_val, VALUE_SIZE);
    value = std::string(rand_val);
  }

  op.__set_value(clientEncrypt(value));

  return op;
}

std::istream& readFile(std::ifstream &seed_data, std::string &line) {
  std::lock_guard<std::mutex> lock(fileMutex);
  return std::getline(seed_data, line);
}

std::string clientEncrypt(const std::string& value) {
  encryption_engine engine;

	std::unique_ptr<unsigned char> cipher_text(new unsigned char[4096]);
	size_t out_len = (size_t) engine.encryptNonDeterministic(value, cipher_text.get());
	std::string updated_val((const char *) cipher_text.get(), out_len);
  return updated_val;
}