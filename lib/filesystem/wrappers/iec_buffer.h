#ifndef MEATFILESYSTEM_WRAPPERS_IEC_BUFFER
#define MEATFILESYSTEM_WRAPPERS_IEC_BUFFER

#include "buffered_io.h"
#include "line_reader_writer.h"
#include "iec.h"
#include "iec_buffer.h"


/********************************************************
 * U8Char
 * 
 * A minimal wide char implementation that can handle UTF8
 * and convert it to PETSCII
 ********************************************************/

class U8Char {
    static const char16_t utf8map[];

    const char missing = '?';

    void fromUtf8Stream(std::istream* reader) {
        uint8_t byte = reader->get();
        if(byte<=0x7f) {
            ch = byte;
        }   
        else if((byte & 0b11100000) == 0b11000000) {
            uint16_t hi =  ((uint16_t)(byte & 0b1111)) << 6;
            uint16_t lo = (reader->get() & 0b111111);
            ch = hi | lo;
        }
        else if((byte & 0b11110000) == 0b11100000) {
            uint16_t hi = ((uint16_t)(byte & 0b111)) << 12;
            uint16_t mi = ((uint16_t)(reader->get() & 0b111111)) << 6;
            uint16_t lo = reader->get() & 0b111111;
            ch = hi | mi | lo;
        }
        else {
            ch = 0;
        }
    };

public:
    char16_t ch;
    U8Char(uint16_t codepoint): ch(codepoint) {};
    U8Char(std::istream* reader) {
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
 * oiecbuf
 * 
 * A buffer for writing IEC data, handles sending EOI
 ********************************************************/

class oiecbuf : public std::filebuf {
    char* data;
    IEC* m_iec;

    size_t easyWrite(bool lastOne) {
        size_t written = 0;

        // we're always writing without the last character in buffer just to be able to send this special delay
        // if this is last character in the file
        Debug_printv("IEC easyWrite writes:");

        for(auto b = pbase(); b<pptr()-1; b++) {
            Serial.printf("%c",*b);
            if(m_iec->send(*b)) written++;
            else
                break;
        }

        if(lastOne) {
            // ok, so we have last character, signal it
            Debug_printv("IEC easyWrite writes THE LAST ONE with EOI:%s", *(pptr()-1));

            m_iec->sendEOI(*(pptr()-1));
            setp(data, data+1024);
        }
        else {
            // let's shift the buffer to the last character
            setp(pptr()-1, data+1024);
        }
    }

public:
    oiecbuf(IEC* iec) : m_iec(iec) {
        data = new char[1024];
        setp(data, data+1024);
    };

    ~oiecbuf() {
        if(data != nullptr)
            delete[] data;

        close();
    }

    bool is_open() const {
        return true;
    }

    int overflow(int ch  = traits_type::eof()) override {
        char* end = pptr();
        if ( ch != EOF ) {
            *end ++ = ch;
        }

        auto written = easyWrite(false);

        if ( written == 0 ) {
            ch = EOF;
        } else if ( ch == EOF ) {
            ch = 0;
        }
        
        return ch;
    };


    int sync() { 
        if(pptr() == pbase()) {
            return 0;
        }
        else {
            auto result = easyWrite(true); 
            return (result != 0) ? 0 : -1;  
        }  
    };
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
        buff = new oiecbuf(i);
    };

    ~oiecstream() {
        delete buff;
    }

    void writeAsPetscii(std::string line) {
        // line is utf-8, convert to petscii

        std::string converted;
        std::stringstream ss(line);

        while(!ss.eof()) {
            U8Char codePoint(&ss);
            converted+=codePoint.toPetscii();
        }

        Debug_printv("UTF8 converted to PETSCII:%s",converted.c_str());

        (*this) << converted;
    }
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
