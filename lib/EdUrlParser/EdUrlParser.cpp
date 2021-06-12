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

#include "EdUrlParser.h"

#define CHECK_LEN_END(POS, LEN) if(POS>=LEN) {_url_errorno=100;goto __PARSE_END;}
#define WALK_SP(POS, LEN, BUF) for(;POS<LEN && BUF[POS]==' ';POS++)
#define WALK_UNTIL(POS, LEN, BUF, DELC) for(;POS<LEN && BUF[POS]!=DELC;POS++)
#define WALK_UNTIL2(POS, LEN, BUF, DELI1, DELI2) for(;POS<LEN && BUF[POS]!=DELI1 && BUF[POS]!=DELI2 ;POS++)
#define WALK_UNTIL3(POS, LEN, BUF, DELI1, DELI2, DELI3) for(;POS<LEN && BUF[POS]!=DELI1 && BUF[POS]!=DELI2 && BUF[POS]!=DELI3;POS++)
#define CHECK_REMAIN_END(POS, LEN, REQ_LEN) if(LEN-POS < REQ_LEN) {_url_errorno=100; goto __PARSE_END; }
#define WALK_CHAR(POS, BUF, DELI) if(BUF[POS++] != DELI) goto __PARSE_END

int __kv_callback_map(void* list, std::string k, std::string v);
int __kv_callback_vec(void* list, std::string k, std::string v);

EdUrlParser::EdUrlParser() {
}

EdUrlParser::~EdUrlParser() {
}

std::string EdUrlParser::urlEncode(std::string s) 
{
    const char *ptr = s.c_str();
    std::string enc;
    char c;
    char phex[3] = { '%' };
    for (size_t i = 0; i < s.size(); i++) {
        c = ptr[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '*'
                || c == '.') {
            enc.push_back(c);
        } else if (c == ' ') {
            enc.push_back('+');
        } else {
            toHex(phex + 1, c);
            enc.append(phex, 0, 3);
        }
    }
    return enc;
}

std::string EdUrlParser::urlDecode(std::string str) 
{
    int _url_errorno = 0;
    size_t pos = 0, per = 0;
    size_t len = str.size();
    const char* buf = str.c_str();
    std::string decstr;
    _url_errorno = 0;
    for (per = pos = 0;;) {
        WALK_UNTIL2(pos, len, buf, '%', '+');
        decstr.append(buf, per, pos - per);
        if (pos >= len)
            goto __PARSE_END;
        if (buf[pos] == '%') {
            CHECK_REMAIN_END(pos, len, 3);
            // try {
                char c = EdUrlParser::toChar(buf + pos + 1);
                decstr.push_back(c);
                pos += 3;
                per = pos;
            // } catch (int err) {
            // 	_url_errorno = err;
            // 	goto __PARSE_END;
            // }
            if (pos >= len)
                goto __PARSE_END;
        } else if (buf[pos] == '+') {
            decstr.push_back(' ');
            pos++;
            per = pos;
        }
    }
    __PARSE_END: if (_url_errorno != 0)
        return "";
    return decstr;

}

void EdUrlParser::toHex(char* desthex, char c) 
{
    static char hextable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
            '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    desthex[0] = hextable[c >> 4];
    desthex[1] = hextable[c & 0x0f];
}

char EdUrlParser::toChar(const char* hex) 
{
    unsigned char nible[2];
    unsigned char c, base;
    for (int i = 0; i < 2; i++) {
        c = hex[i];
        if (c >= '0' && c <= '9') {
            base = '0';
        } else if (c >= 'A' && c <= 'F') {
            base = 'A' - 10;
        } else if (c >= 'a' && c <= 'f') {
            base = 'a' - 10;
        } else {
            //throw 200;
        }
        nible[i] = c - base;
    }
    return ((nible[0] << 4) | nible[1]);
}


void EdUrlParser::parseUrl(std::string src_url) 
{
    Serial.printf("\nparseUrl: [%d][%s]\n", src_url.size(), src_url.c_str());
    int _url_errorno = 0;
    const char *str = src_url.c_str();

    size_t pos, len;
    size_t scheme_pos, user_pos, pass_pos, host_pos, port_pos, path_pos, param_pos, tag_pos;
    pos = 0;
    len = src_url.size();
    WALK_SP(pos, len, str); // remove preceding spaces.
    if (str[pos] == '/') {
        goto __PARSE_HOST;
    }

    // start protocol scheme
    scheme_pos = pos;
    WALK_UNTIL(pos, len, str, ':');
    CHECK_LEN_END(pos, len);
    scheme = src_url.substr(scheme_pos, pos - scheme_pos);
    CHECK_REMAIN_END(pos, len, 3);
    WALK_CHAR(pos, str, ':');
    WALK_CHAR(pos, str, '/');
    WALK_CHAR(pos, str, '/');

    // start username / password
    user_pos = pos;
    WALK_UNTIL(pos, len, str, '@');
    if (pos == len)
    {
        pos = user_pos;
        goto __PARSE_HOST;
    }
    host_pos = pos;
    pos = user_pos;
    WALK_UNTIL(pos, len, str, ':');
    if (pos > host_pos)
    {
        // Username Only
        pos = user_pos;
        WALK_UNTIL(pos, len, str, '@');
        username = src_url.substr(user_pos, pos - user_pos);
        WALK_CHAR(pos, str, '@');
    }
    else
    {
        // Username & Password
        username = src_url.substr(user_pos, pos - user_pos);
        WALK_CHAR(pos, str, ':');
        pass_pos = pos;
        WALK_UNTIL(pos, len, str, '@');
        password = src_url.substr(pass_pos, pos - pass_pos);
        WALK_CHAR(pos, str, '@');
    }

    // start host address
    __PARSE_HOST:
    host_pos = pos;
    WALK_UNTIL3(pos, len, str, ':', '/', '?');
    if (pos < len) {
        hostname = src_url.substr(host_pos, pos - host_pos);
        if (str[pos] == ':')
            goto __PARSE_PORT;
        if (str[pos] == '/')
            goto __PARSE_PATH;
        if (str[pos] == '?')
            goto __PARSE_PARAM;
    } else {
        hostname = src_url.substr(host_pos, pos - host_pos);
    }

    __PARSE_PORT:
    WALK_CHAR(pos, str, ':');
    port_pos = pos;
    WALK_UNTIL2(pos, len, str, '/', '?');
    port = src_url.substr(port_pos, pos - port_pos);
    CHECK_LEN_END(pos, len);
    if (str[pos] == '?')
        goto __PARSE_PARAM;

    __PARSE_PATH: path_pos = pos;
    WALK_UNTIL(pos, len, str, '?');
    path = src_url.substr(path_pos, pos - path_pos);
    CHECK_LEN_END(pos, len);

    __PARSE_PARAM:
    WALK_CHAR(pos, str, '?');
    param_pos = pos;
    WALK_UNTIL(pos, len, str, '#');
    query = src_url.substr(param_pos, pos - param_pos);
    CHECK_LEN_END(pos, len);

    // start parsing fragment
    WALK_CHAR(pos, str, '#');
    tag_pos = pos;
    fragment = src_url.substr(tag_pos, len - tag_pos);

    __PARSE_END:
        replace_all(path, "//", "/");
        parsePath(path);
        return;
}

size_t EdUrlParser::parsePath(std::vector<std::string>* folders, std::string pathstr) 
{
    int _url_errorno = 0;
    int path_pos = 0;
    size_t pos = 0;
    size_t len = pathstr.size();
    const char* str = pathstr.c_str();
    std::string name;
    for (pos = 0;;) {
        WALK_CHAR(pos, str, '/');
        path_pos = pos;
        CHECK_LEN_END(pos, len);
        WALK_UNTIL(pos, len, str, '/');
        name = pathstr.substr(path_pos, pos - path_pos);
        folders->push_back(name);
    }
    __PARSE_END:
        return folders->size();
}

void EdUrlParser::parsePath(std::string pathstr) 
{
    size_t pos = 0;

    // Path
    pos = pathstr.find_last_of('/');
    path = pathstr.substr(0, pos) + '/';

    // File
    filename = pathstr.substr(pos + 1);

    // Extension
    pos = filename.find_last_of('.');
    if ( pos < filename.size() )
    {
        extension = filename.substr(pos + 1);
        string_toupper(extension);
    }
}

std::string EdUrlParser::root(void)
{
    string_toupper(scheme);
    
    // set root URL
    std::string root = scheme + "://";
    if ( username.size() )
    {
        root += username;
        if ( password.size() )
            root += ':' + password;
        root += '@';
    }
    root += hostname;
    if ( port.size() )
        root += ':' + port;

    return root;
}

std::string EdUrlParser::base(void)
{
    // set base URL
    return root() + path;
}

std::string EdUrlParser::url(void)
{
    // set full URL
    std::string url = base();
    url += filename;
    if ( query.size() )
        url += '?' + query;
    if ( fragment.size() )
        url += '#' + fragment;

    return url;
}

size_t EdUrlParser::parseKeyValueMap(std::unordered_map<std::string, std::string> *kvmap, std::string rawstr, bool strict) {
    return parseKeyValue(rawstr, __kv_callback_map, kvmap, strict);
}

size_t EdUrlParser::parseKeyValueList(std::vector< query_kv_t > *kvvec, std::string rawstr, bool strict) {
    return parseKeyValue(rawstr, __kv_callback_vec, kvvec, strict);
}

size_t EdUrlParser::parseKeyValue(std::string rawstr, __kv_callback kvcb, void* obj, bool strict) {

    int _url_errorno = 0;
    const char *str = rawstr.c_str();
    size_t pos, len, item_len;
    pos = 0;
    len = rawstr.size();

    std::string key, val;
    size_t key_pos;
    WALK_SP(pos, len, str);
    CHECK_LEN_END(pos, len);
    key_pos = pos;
    item_len = 0;
    for(;;) {
        WALK_UNTIL2(pos, len, str, '=', '&');
        if(pos >= len || str[pos] == '&') {
            // Be careful for boundary check error to be caused. !!!
            // *** Do not access str[] any more in this block. !!!

            val = rawstr.substr(key_pos, pos-key_pos);

            if(strict == true) {
                if(key.empty() == false && val.empty()==false) {
                    kvcb(obj, key, val);
                    item_len++;
                }
            } else if(!(key.empty()==true && val.empty()==true)){
                kvcb(obj, key, val);
                item_len++;
            }

            key.clear();val.clear();
            if(pos >= len) goto __PARSE_END;
            pos++;
            key_pos = pos;
        }
        else if(str[pos] == '=') {
            key = rawstr.substr(key_pos, pos-key_pos);
            pos++;
            key_pos = pos;
        }
    }
    __PARSE_END:
    if(_url_errorno != 0 )
        return -1;
    return item_len;
}


int __kv_callback_map(void* list, std::string k, std::string v) {
    auto *map = (std::unordered_map<std::string, std::string>*)list;
    (*map)[k] = v;
    return map->size();
}

int __kv_callback_vec(void* list, std::string k, std::string v) {
    auto *vec = (std::vector<query_kv_t>*)list;
    query_kv_t t ={k, v};
    vec->push_back(t);
    return vec->size();
}

// convert to uppercase (in place)
void EdUrlParser::string_toupper(std::string &s)
{
    transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return toupper(c); });
}

void EdUrlParser::replace_all(std::string &str, const std::string &from, const std::string &to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}