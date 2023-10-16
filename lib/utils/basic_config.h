#ifndef MEATLOAF_UTILS_BASIC_CONFIG
#define MEATLOAF_UTILS_BASIC_CONFIG

#include <cstring>
#include <unordered_map>

#include "meat_io.h"
#include "meat_buffer.h"


class BasicConfigReader {
public:
    std::unordered_map<std::string, std::string>* entries;

    BasicConfigReader() {
        entries = new std::unordered_map<std::string, std::string>();
    }

    ~BasicConfigReader() {
        delete entries;
    }

    void read(std::string name);
    std::string get(std::string key);
};

#endif /* MEATLOAF_UTILS_BASIC_CONFIG */
