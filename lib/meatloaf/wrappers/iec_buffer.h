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
    bool m_isOpen = false;

    size_t easyWrite(bool lastOne);

public:
    oiecbuf() {
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
        return m_isOpen;
    }

    virtual void open(IEC* iec) {
        m_iec = iec;
        if(iec != nullptr)
            m_isOpen = true;
    }

    virtual void close() {
        sync();
        m_isOpen = false;
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
    oiecbuf buff;

public:
    oiecstream() : std::ostream(&buff) {
        Debug_printv("oiecstream constructor");
    };

    ~oiecstream() {
        Debug_printv("oiecstream destructor");
    }

    void putUtf8(U8Char* codePoint);

    void open(IEC* i) {
        buff.open(i);
    }

    void close() {
        buff.close();
    }

    virtual bool is_open() {
        return buff.is_open();
    }
};

#endif /* MEATFILESYSTEM_WRAPPERS_IEC_BUFFER */
