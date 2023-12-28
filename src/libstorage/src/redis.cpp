#include "redis.h"

redisCli::redisCli(const std::string &redis_ip, int redis_port) {
    connection_options.host = redis_ip;
    connection_options.port = redis_port;

    redisConn = std::make_shared<Redis>(connection_options);
}

std::string redisCli::get(const std::string &key) {
    return redisConn->get(key).value_or("");
}

void redisCli::put(const std::string &key, const std::string &value) {
    redisConn->set(key, value);
}

void redisCli::put_batch(const std::vector<std::pair<std::string, std::string>> &operations) {
    auto pipe = redisConn->pipeline();
    for (auto &op : operations) {
        pipe.set(op.first, op.second);
    }
    pipe.exec();
}