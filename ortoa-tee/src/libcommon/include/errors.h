#ifndef ERROR_H
#define ERROR_H

#include <string>

class OECreationFailed {
    std::string message;
public:
    OECreationFailed(std::string msg): message{msg} {};
    std::string what() { return "oe_create_ortoa_enclave() failed with enclave path " + message; }
};

#endif
