#ifndef MEATFILE_PUP_H
#define MEATFILE_PUP_H


#include <string>
#include <vector>
#include <sstream>
#include "utils.h"
#include "string_utils.h"

class PeoplesUrlParser {
public:
    std::string scheme;
    std::string path;
    std::string user;
    std::string pass;
    std::string host;
    std::string port;
    std::string url;
    std::string name;
    std::string extension;
private:

    void parseHostPortPath(std::string userPass) {
        Serial.printf("parseHostPortPath [%s]\n", userPass.c_str());
        // host.com:port/path/path/path....
        auto parts = mstr::split(userPass,':', 2);

        if(parts.size()>1) {
            host = parts[0];
            auto portPath = mstr::split(parts[1],'/',2);

            port = portPath[0];
            path = portPath[1];
        }
        else {
            auto hostPath = mstr::split(parts[0],'/',2);
            if(hostPath.size()>1)
            {
                host = hostPath[0];
                path = hostPath[1];                
            }
        }
    }

    void parseUserPass(std::string userPass) {
        // user:pass

        auto parts = mstr::split(userPass,':', 2);

        user = parts[0];

        if(parts.size()>1) {
            pass = parts[1];
        }
    }

    void parseAuthorityPath(std::string authPath) {
        bool hasAuthority = authPath[0]=='/' && authPath[1]=='/';

        std::string authPathStripped = (hasAuthority) ? authPath.substr(2) : authPath;
        if(hasAuthority) {
            auto parts = mstr::split(authPathStripped, '@', 2);
            std::string hostPortPath;

            if(parts.size()>1) {
                // we have user/pass part
                parseUserPass(parts[0]); // user:pass
                hostPortPath = parts[1]; // host.com:port/path/path/path....
            }
            else {
                // we just have host/port
                hostPortPath = parts[0]; // host.com:port/path/path/path....
            }
            parseHostPortPath(hostPortPath);
        }

        auto firstSlash = authPathStripped.find('/');

//Serial.printf("auth path stripped=%s\n",authPathStripped.c_str());

        if(firstSlash != std::string::npos)
            path = authPathStripped.substr(firstSlash);
        else
            path = authPathStripped;
    }

public:
    void parseUrl(std::string u) {
        url = u;
        auto parts = mstr::split(url, ':', 2);

        scheme = "";
        path = "";
        user = "";
        pass = "";
        host = "";
        port = "";

        Serial.printf("url [%s] parts.size [%d]\n", u.c_str(), parts.size());
        if(parts.size()>1) {
            scheme = parts[0]; // http
            parseAuthorityPath(parts[1]); // user:pass@host.com:port/path/path/path....
        }
        else {
            scheme = "";
            parseAuthorityPath(parts[0]); // user:pass@host.com:port/path/path/path....
        }

        auto pathParts = mstr::split(path,'/');

//Serial.printf("path parts %s\n", mstr::joinToString(pathParts, "::").c_str());

        //auto last = (pathParts.end())--;
        
        if(pathParts.size()>1)
            name = *(--pathParts.end());
        else
            name = path;

        auto nameParts = mstr::split(name,'.');
        
        //last = (nameParts.end())--;
        if(nameParts.size()>1)
            extension = *(--nameParts.end());
        else
            extension = "";
    }

    // void dump() {
    //     printf("scheme: %s\n", scheme.c_str());
    //     printf("host port: %s -- %s\n", host.c_str(), port.c_str());
    //     printf("path: %s\n", path.c_str());
    //     printf("user pass: %s -- %s\n", user.c_str(), pass.c_str());
    // }


};

#endif