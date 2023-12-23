#ifndef REDIS_H
#define REDIS_H

#include <iostream>
#include <sw/redis++/redis++.h>

#include "StorageInterface.h"

class redisCli : public StorageInterface
{
    public:
        redisCli();
        std::string get(const std::string &key) override;
        void put(const std::string &key, const std::string &value) override;
        sw::redis::Pipeline pipe();
        void reconnect();

    private:
        sw::redis::Redis redisConn = sw::redis::Redis("tcp://127.0.0.1:6379");
};

#endif //REDIS_H
