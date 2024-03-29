#ifndef STORAGE_INTERFACE_H
#define STORAGE_INTERFACE_H

#include <string>

class StorageInterface {
  public:
    virtual std::string get(const std::string &key) = 0;
    virtual void put(const std::string &key, const std::string &value) = 0;
    virtual void put_batch(const std::vector<std::pair<std::string, std::string>> &operations) = 0;
};

#endif
