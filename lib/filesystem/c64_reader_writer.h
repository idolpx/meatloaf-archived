#ifndef MEATFILE_C64STREAM_WRITER_H
#define MEATFILE_C64STREAM_WRITER_H

#include "buffered_io.h"
#include "line_reader_writer.h"


class U8Char {
    static const char16_t utf8map[];

    const char missing = '?';

    void fromUtf8Stream(StreamReader* reader) {
        uint8_t byte = reader->readByte();
        if(byte<=0x7f) {
            ch = byte;
        }
        else if((byte & 0b11100000) == 0b11000000) {
            uint16_t hi =  ((uint16_t)(byte & 0b1111)) << 6;
            uint16_t lo = (reader->readByte() & 0b111111);
            ch = hi | lo;
        }
        else if((byte & 0b11110000) == 0b11100000) {
            uint16_t hi = ((uint16_t)(byte & 0b111)) << 12;
            uint16_t mi = ((uint16_t)(reader->readByte() & 0b111111)) << 6;
            uint16_t lo = reader->readByte() & 0b111111;
            ch = hi | mi | lo;
        }
        else {
            ch = 0;
        }
    };

public:
    char16_t ch;
    U8Char(uint16_t codepoint): ch(codepoint) {};
    U8Char(StreamReader* reader) {
        fromUtf8Stream(reader);
    }
    
    U8Char(char petscii) {
        ch = utf8map[(uint8_t)petscii];
    }

    std::string toUtf8() {
        if(ch==0) {
            return std::string(1, missing);
        }
        else if(ch>=0x01 && ch<=0x7f) {
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

    uint8_t toPetscii() {
        for(int i = 0; i<256; i++) {
            if(utf8map[i]==ch)
                return i;
        }
        return missing;
    }
};


/********************************************************
 * Stream writer
 * 
 *  We have some UTF8 text and want to save it to PETSCII
 ********************************************************/

class C64StreamWriter: public StreamWriter {
public:
    char delimiter = '\n';

    C64StreamWriter(MOstream* os) : StreamWriter(os) { 
    };

    bool print(std::string line) {
        // line is utf-8, convert to petscii
        StreamReader* a; // TODO create string reader!

        std::string converted;
        while(!a->eof()) {
            U8Char codePoint(a);
            converted+=codePoint.toPetscii();
        }
        MBufferConst buffer(converted);
        return write(&buffer);        
    }

    bool printLn(std::string line) {
        return print(line+delimiter); 
    }
};

/********************************************************
 * Stream reader
 * 
 * We have some PETSCII on C64, we need to get it in UTF8
 ********************************************************/

class C64StreamReader: public StreamReader {
    int buffPos;
    std::string lineBuilder;

public:
    char delimiter = '\n';

    C64StreamReader(MIstream* is) : StreamReader(is), buffPos(0), lineBuilder("") { };
    
    C64StreamReader(const std::function<int(uint8_t* buf, size_t size)>& fn) : StreamReader(fn), buffPos(0), lineBuilder("") {}

    std::string readLn() {
        auto line = StreamReader::readLn();

        std::string converted;

        for(int i = 0; i<line.length(); i++) {
            converted+=U8Char(smartBuffer[buffPos]).toUtf8();
        }

        return converted;
    };

};

#endif