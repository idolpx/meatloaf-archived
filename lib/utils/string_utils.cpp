#include "string_utils.h"

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

}