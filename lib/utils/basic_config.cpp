#include "basic_config.h"

std::string BasicConfigReader::get(std::string key) {
    if(entries->find(key) == entries->end())
        return nullptr;
    else
        return entries->at(key);
}

void BasicConfigReader::read(std::string name) {
    Meat::iostream istream(name);

    if(istream.is_open()) {
        // skip two bytes:
        // LH - load address

        while(!istream.eof()) {
            uint16_t word;
            std::string line;
            // parse line here:
            // LH - next line ptr, if both == 0 - stop here, no more lines
            istream >> word;
            if(word != 0) {
                // LH - line number
                istream >> word;
                // config_name:config value(s)
                // 0
                istream >> line;
                mstr::toASCII(line);
                auto split = mstr::split(line, ':', 2);
                if(split.size()>1) {
                    auto setting = mstr::drop(split[0],1); // drop opening quote
                    auto value = split[1];
                    mstr::trim(setting); // drop whitespace
                    mstr::trim(value);

                    (*entries)[setting] = value;
                }
            }
        }

        istream.close();
    }
}