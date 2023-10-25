#ifndef REDIS_H
#define REDIS_H

#include <iostream>
#include <sw/redis++/redis++.h>

class redisCli
{
    public:
        redisCli();
        std::string get(const std::string &key);
        void put(const std::string &key, const std::string &value);
        sw::redis::Pipeline pipe();
        void reconnect();

    private:
        sw::redis::Redis redisConn = sw::redis::Redis("tcp://127.0.0.1:6379");
};

#endif //REDIS_H
