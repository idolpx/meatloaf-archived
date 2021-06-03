

#ifndef URLPARSER_H_
#define URLPARSER_H_

#include <string>

#include "utils.h"

class URLParser 
{
private:
	URLParser();
	virtual ~URLParser();

	void parse();

public:
	void parsePath(std::string pathstr);

	static std::string urlEncode(std::string s);	
	static std::string urlDecode(std::string str);
	static void toHex(char *desthex, char c);
	static char toChar(const char* hex);
	static URLParser* parseUrl(std::string urlstr);
	static std::string parseKeyValue(std::string rawstr, std::string key);

	std::string url;
	std::string root;
    std::string base;
	std::string scheme;
	std::string username;
	std::string password;
	std::string host;
	std::string port;
	std::string path;
    std::string file;
    std::string extension;
	std::string query;
	std::string fragment;
};

#endif /* URLPARSER_H_ */
