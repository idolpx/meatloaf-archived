#ifndef MEATFILE_C64STREAM_WRITER_H
#define MEATFILE_C64STREAM_WRITER_H

#include "buffered_io.h"
#include "line_reader_writer.h"


/********************************************************
 * U8Char
 * 
 * A minimal wide char implementation that can handle UTF8
 * and convert it to PETSCII
 ********************************************************/

class U8Char {
    static const char16_t utf8map[];

    const char missing = '?';

    void fromUtf8Stream(LinedReader* reader) {
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
    U8Char(LinedReader* reader) {
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
            auto upper = (ch>>6) & (0b11111 | 0b11000000); 
            char lower = ch & (0b111111 | 0b10000000); 
            char arr[] = { (char)upper, (char)lower, '\0'};
            return std::string(arr);
        }
        else {
            auto lower = (uint8_t)ch & (0b00111111 | 0b10000000);
            auto mid = (uint8_t)(ch>>6) & (0b00111111 | 0b10000000);
            auto hi = (uint8_t)(ch>>12) & (0b00111111 | 0b11100000);
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
 * For writing UTF8 streams to PETSCII-talking devices
 ********************************************************/

class C64LinedWriter: public LinedWriter {
public:
    char delimiter = '\n';

    C64LinedWriter(MOStream* os) : LinedWriter(os) { 
    };

    bool print(std::string line) {
        // line is utf-8, convert to petscii
        LinedReader* a; // TODO create string reader!

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
 * For reading PETSCII encoded steams into UTF8 lines of text
 ********************************************************/

class C64LinedReader: public LinedReader {
    int buffPos;
    std::string lineBuilder;

public:
    char delimiter = '\n';

    C64LinedReader(MIStream* is) : LinedReader(is), buffPos(0), lineBuilder("") { };
    
    C64LinedReader(const std::function<int(uint8_t* buf, size_t size)>& fn) : LinedReader(fn), buffPos(0), lineBuilder("") {}

    std::string readLn() {
        auto line = LinedReader::readLn();

        std::string converted;

        for(size_t i = 0; i<line.length(); i++) {
            converted+=U8Char(smartBuffer[buffPos]).toUtf8();
        }

        return converted;
    };

};

#endif