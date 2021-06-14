#include "string_utils.h"
#include <algorithm>


namespace mstr {
    bool compare_char(char &c1, char &c2)
    {
        if (c1 == c2)
            return true;

        return false;
    }

    bool compare_char_insensitive(char &c1, char &c2)
    {
        if (c1 == c2)
            return true;
        else if (std::toupper(c1) == std::toupper(c2))
            return true;
        return false;
    }



    // trim from start (in place)
    void ltrim(std::string &s)
    {
        s.erase(
            s.begin(),
            std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
    }

    // trim from end (in place)
    void rtrim(std::string &s)
    {
        s.erase(
            std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    }

    // trim from both ends (in place)
    void trim(std::string &s)
    {
        ltrim(s);
        rtrim(s);
    }



    std::string drop(std::string str, int count) {
        if(count>str.length())
            return "";
        else
            return str.substr(count);
    }

    std::string dropLast(std::string str, int count) {
        if(count>str.length())
            return "";
        else
            return str.substr(0, str.length()-count);
    }

    bool startsWith(std::string s, const char *pattern, bool case_sensitive)
    {
        if (s.empty() && pattern == nullptr)
            return true;
        if (s.empty() || pattern == nullptr)
            return false;
        if(s.length()<strlen(pattern))
            return false;

        std::string ss = s.substr(0, strlen(pattern));
        std::string pp = pattern;

        return equals(ss, pp, case_sensitive);
    }

    bool endsWith(std::string s, const char *pattern, bool case_sensitive)
    {
        if (s.empty() && pattern == nullptr)
            return true;
        if (s.empty() || pattern == nullptr)
            return false;
        if(s.length()<strlen(pattern))
            return false;

        std::string ss = s.substr((s.length() - strlen(pattern)));
        std::string pp = pattern;

        return equals(ss, pp, case_sensitive);
    }


    /*
    * String Comparision
    */
    bool equals(std::string &s1, std::string &s2, bool case_sensitive)
    {
        if(case_sensitive)
            return ( (s1.size() == s2.size() ) &&
                std::equal(s1.begin(), s1.end(), s2.begin(), &compare_char) );
        else
            return ( (s1.size() == s2.size() ) &&
                std::equal(s1.begin(), s1.end(), s2.begin(), &compare_char_insensitive) );

    }

    // convert to lowercase (in place)
    void toLower(std::string &s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    }

    // convert to uppercase (in place)
    void toUpper(std::string &s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                    [](unsigned char c) { return std::toupper(c); });
    }



    std::vector<std::string> split(std::string toSplit, char ch, int limit) {
        std::vector<std::string> parts;

        limit--;

        while(limit > 0 && toSplit.size()>0) {
            auto pos = toSplit.find(ch);
            if(pos == std::string::npos) {
                parts.push_back(toSplit);
                return parts;
            }
            parts.push_back(toSplit.substr(0, pos));

            toSplit = toSplit.substr(pos+1);

            limit--;
        }
        parts.push_back(toSplit);

        return parts;
    }

    std::string joinToString(std::vector<std::string>::iterator* start, std::vector<std::string>::iterator* end, std::string separator) {
        std::string res;

        if((*start)>=(*end))
            return std::string();

        for(auto i = (*start); i<(*end); i++) {
            res+=(*i);
            if(i<(*end))
                res+=separator;
        }

        return res.erase(res.length()-1,1);
    }

    std::string joinToString(std::vector<std::string> strings, std::string separator) {
        auto st = strings.begin();
        auto ed = strings.end();
        return joinToString(&st, &ed, separator);
    }

}