/*
    Copyright (c) 2014 netminds, https://github.com/netmindms/urlparser
    Copyright (c) 2021 James Johnston, https://github.com/idolpx/urlparser
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. The name of the author may not be used to endorse or promote products
       derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
    SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
    OF SUCH DAMAGE.
*/

#ifndef EDURLPARSER_H_
#define EDURLPARSER_H_

#include "../../include/global_defines.h"

#include <unordered_map>
#include <tuple>
#include <vector>
#include <string>
#include <algorithm>

typedef struct {
    std::string key;
    std::string val;
} query_kv_t;

typedef int (*__kv_callback)(void* list, std::string k, std::string v);

class EdUrlParser 
{
private:
    void string_toupper(std::string &s);

public:
    EdUrlParser();
    ~EdUrlParser();

    void parsePath(std::string pathstr);
    int parsePath(std::vector<std::string>* folders, std::string pathstr);
    void parseUrl(std::string urlstr);

    static std::string urlEncode(std::string s);	
    static std::string urlDecode(std::string str);
    static void toHex(char *desthex, char c);
    static char toChar(const char* hex);
    
    static size_t parseKeyValueMap(std::unordered_map<std::string, std::string> *kvmap, std::string str, bool strict=true);
    static size_t parseKeyValueList(std::vector< query_kv_t > *kvmap, std::string rawstr, bool strict=true);
    static size_t parseKeyValue(std::string rawstr, __kv_callback kvcb, void* obj, bool strict);

    std::string url;
    std::string root;
    std::string base;
    std::string scheme;
    std::string username;
    std::string password;
    std::string hostname;
    std::string port;
    std::string pathX;
    std::string filename;
    std::string extension;
    std::string query;
    std::string fragment;
};

#endif /* EDURLPARSER_H_ */
