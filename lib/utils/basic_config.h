#ifndef MEATLIB_UTILS_BASIC_CONFIG
#define MEATLIB_UTILS_BASIC_CONFIG

#include "string_utils.h"
#include <map>
#include "meat_io.h"


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

#endif /* MEATLIB_UTILS_BASIC_CONFIG */
