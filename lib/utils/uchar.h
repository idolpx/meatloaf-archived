#include <memory>
#include <string>

class Utf8 {
    char16_t ch;

    void toChar16(const std::function<uint8_t()>& getByte) {
        uint8_t byte = getByte(); // getByte()
        if((byte >> 7) == 0)
            // 0x00 do 0x7F            – bity 0xxxxxxx, gdzie kolejne „x” to bity – licząc od najwyższego
            ch = byte;
        else if((byte >> 5) == 0b110) {
            // 0x80 do 0x7FF           – bity 110xxxxx 10xxxxxx
            uint16_t part0 = (byte & 0b11111) << 6;
            uint16_t part1 = getByte() & 0b111111;
            ch = part0 | part1;

        }
        else if((byte >> 4) == 0b1110) {
            // 0x800 do 0xFFFF         – bity 1110xxxx 10xxxxxx 10xxxxxx
            uint16_t part0 = (byte & 0b1111) << 12;
            uint16_t part1 = (getByte() & 0b111111) << 6;
            uint16_t part2 = getByte() & 0b111111;
            ch = part0 | part1 | part2;
        }
        else
            ch = 0;
    };

    std::string toUtf8() {
        if(ch>=0x00 && ch<=0x7f) {
            // 0x00 do 0x7F            – bity 0xxxxxxx, gdzie kolejne „x” to bity – licząc od najwyższego
            //return std::string((char)ch);
        }
        else if(ch>=0x80 && ch<=0x7ff) {
            // 0x80 do 0x7FF           – bity 110xxxxx 10xxxxxx

        }
        else {
            // 0x800 do 0xFFFF         – bity 1110xxxx 10xxxxxx 10xxxxxx

        }
    }
};