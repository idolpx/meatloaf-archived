#ifndef MEATFILESYSTEM_WRAPPERS_IEC_BUFFER
#define MEATFILESYSTEM_WRAPPERS_IEC_BUFFER

#include "iec.h"
#include "iec_buffer.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "U8Char.h"


/********************************************************
 * oiecbuf
 * 
 * A buffer for writing IEC data, handles sending EOI
 ********************************************************/

class oiecbuf : public std::filebuf {
    char* data;
    IEC* m_iec;

    size_t easyWrite(bool lastOne);

public:
    oiecbuf(IEC* iec) : m_iec(iec) {
        Debug_printv("oiecbuffer constructor");

        data = new char[1024];
        setp(data, data+1024);
    };

    ~oiecbuf() {
        Debug_printv("oiecbuffer desttructor");
        if(data != nullptr)
            delete[] data;

        close();
    }

    bool is_open() const {
        return true;
    }

    virtual void close() {
        sync();
    }

    int overflow(int ch  = traits_type::eof()) override;

    int sync() override;
};


/********************************************************
 * oiecstream
 * 
 * Standard C++ stream for writing to IEC
 ********************************************************/

class oiecstream : public std::ostream {
    oiecbuf* buff;
public:
    oiecstream(IEC* i) : std::ostream(buff) {
        Debug_printv("oiecstream constructor");
        buff = new oiecbuf(i);
    };

    ~oiecstream() {
        Debug_printv("oiecstream destructor");

        delete buff;
    }

    void putUtf8(U8Char* codePoint);

    void close() {
        buff->close();
    }

    // void writeAsPetscii(std::string line) {
    //     // line is utf-8, convert to petscii

    //     std::string converted;
    //     std::stringstream ss(line);

    //     while(!ss.eof()) {
    //         U8Char codePoint(&ss);
    //         converted+=codePoint.toPetscii();
    //     }

    //     Debug_printv("UTF8 converted to PETSCII:%s",converted.c_str());

    //     (*this) << converted;
    // }
};



/********************************************************
 * Stream reader
 * 
 * For reading PETSCII encoded steams into UTF8 lines of text
 ********************************************************/

/*

    std::string readLn() {
        auto line = LinedReader::readLn();

        std::string converted;

        for(size_t i = 0; i<line.length(); i++) {
            converted+=U8Char(smartBuffer[buffPos]).toUtf8();
        }

        return converted;
    };

*/



#endif /* MEATFILESYSTEM_WRAPPERS_IEC_BUFFER */
