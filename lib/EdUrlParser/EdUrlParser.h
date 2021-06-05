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

#include <unordered_map>
#include <tuple>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

typedef struct {
    string key;
    string val;
} query_kv_t;

typedef int (*__kv_callback)(void* list, string k, string v);



class EdUrlParser 
{
private:
    EdUrlParser();


    void parse();
    void string_toupper(std::string &s);

public:
    virtual ~EdUrlParser();

    void parsePath(string pathstr);
    static int parsePath(vector<string>* folders, string pathstr);
    static EdUrlParser* parseUrl(string urlstr);

    static string urlEncode(string s);	
    static string urlDecode(string str);
    static void toHex(char *desthex, char c);
    static char toChar(const char* hex);
    
    static size_t parseKeyValueMap(unordered_map<string, string> *kvmap, string str, bool strict=true);
    static size_t parseKeyValueList(vector< query_kv_t > *kvmap, string rawstr, bool strict=true);
    static size_t parseKeyValue(string rawstr, __kv_callback kvcb, void* obj, bool strict);

    string url;
    string root;
    string base;
    string scheme;
    string username;
    string password;
    string hostname;
    string port;
    string path;
    string filename;
    string extension;
    string query;
    string fragment;
};

#endif /* EDURLPARSER_H_ */
