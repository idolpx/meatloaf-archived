#include <memory>
#include <string>
#include <sstream>

void test() {
    auto kratka = U8Char(0x2592);
    auto asUtfStream = kratka.toUtf8();

auto a = asUtfStream[0];

    std::stringstream ss(asUtfStream);
    auto readFn = [&ss]() {
        char c;
        ss.read(&c, 1);
        return c;
    };

    auto decoded(readFn);

    
}

/*
utf8 to petscii:

std::stringstream sstream(aLineOfText);
auto readFn = [&sstream]() {
    char c;
    ss.read(&c, 1);
    return c;
};

U8Char uchar(readFn);
// in petscii - utf8 table find a utf8 and return petscii

petscii to utf8:

find utf8 u at index p in petscii

U8Char uchar(u);
u.toUtf8(); a string of up to 3 chars

*/

class U8Char {
    //std::u16string str;

    void fromUtf8Stream(const std::function<uint8_t()>& getByte) {
        uint8_t byte = getByte();
        if(byte<=0x7f) {
            ch = byte;
        }
        else if((byte & 0b11100000) == 0b11000000) {
            uint16_t hi =  ((uint16_t)(byte & 0b1111)) << 6;
            uint16_t lo = (getByte() & 0b111111);
            ch = hi | lo;
        }
        else if((byte & 0b11110000) == 0b11100000) {
            uint16_t hi = ((uint16_t)(byte & 0b111)) << 12;
            uint16_t mi = ((uint16_t)(getByte() & 0b111111)) << 6;
            uint16_t lo = getByte() & 0b111111;
            ch = hi | mi | lo;
        }
        else {
            ch = 0;
        }
    };

public:
    char16_t ch;
    U8Char(uint16_t codepoint): ch(codepoint) {};
    U8Char(const std::function<uint8_t()>& getByte) {
        fromUtf8Stream(getByte);
    }

    std::string toUtf8() {
        if(ch>=0x00 && ch<=0x7f) {
            return std::string(1, char(ch));
        }
        else if(ch>=0x80 && ch<=0x7ff) {
            auto upper = (ch>>6) & 0b11111 | 0b11000000; 
            char lower = ch & 0b111111 | 0b10000000; 
            char arr[] = { (char)upper, (char)lower, '\0'};
            return std::string(arr);
        }
        else {
            auto lower = (uint8_t)ch & 0b00111111 | 0b10000000;
            auto mid = (uint8_t)(ch>>6) & 0b00111111 | 0b10000000;
            auto hi = (uint8_t)(ch>>12) & 0b00111111 | 0b11100000;
            char arr[] = { (char)hi, (char)mid, (char)lower, '\0'};
            return std::string(arr);
        }
    }
};