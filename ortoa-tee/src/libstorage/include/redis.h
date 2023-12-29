#ifndef REDIS_H
#define REDIS_H

#include <sw/redis++/redis++.h>

#include "StorageInterface.h"

using namespace sw::redis;

class redisCli : public StorageInterface {
  private:
    ConnectionOptions connection_options;
    std::unique_ptr<Redis> redisConn;

  public:
    redisCli(const std::string &redis_ip, int redis_port = 6379);
    std::string get(const std::string &key) override;
    void put(const std::string &key, const std::string &value) override;
    void put_batch(const std::vector<std::pair<std::string, std::string>> &operations) override;
};

#endif //REDIS_H
