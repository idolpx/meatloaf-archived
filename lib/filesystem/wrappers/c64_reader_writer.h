#ifndef MEATFILESYSTEM_WRAPPERS_C64_READER_WRITER
#define MEATFILESYSTEM_WRAPPERS_C64_READER_WRITER

#include "buffered_io.h"
#include "line_reader_writer.h"
#include "iec.h"


/********************************************************
 * U8Char
 * 
 * A minimal wide char implementation that can handle UTF8
 * and convert it to PETSCII
 ********************************************************/

class U8Char {
    static const char16_t utf8map[];

    const char missing = '?';

    void fromUtf8Stream(BufferedReader* reader) {
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
    U8Char(BufferedReader* reader) {
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
 * IEC MOstream
 * 
 * A nice API for IEC
 ********************************************************/

class IECOstream: public MOStream {
    IEC* m_iec;

    bool sendEOI(uint8_t data) {
        return m_iec->sendEOI(data);
    }

    bool sendFNF() {
        return m_iec->sendFNF();
    }

public:
    IECOstream(IEC* iec): m_iec(iec) {};
    bool seek(uint32_t pos, SeekMode mode) { return false; };
    bool seek(uint32_t pos) { return false; };
    size_t position();
    void close() {};
    bool open() {};
    ~IECOstream() {};
    bool isOpen() {
        return true;
    };

    size_t write(const uint8_t *buf, size_t size);
    size_t writeLast(const uint8_t *buf, size_t size);

        
    void flush() {};

};

/********************************************************
 * String reader
 * 
 * For writing UTF8 streams to PETSCII-talking devices
 ********************************************************/

class StringReader: public BufferedReader {
    std::string buffer;
    size_t pos = 0;

public:
    StringReader(std::string b) : BufferedReader(), buffer(b) {};

    void refillBuffer() override {};
    
    MBuffer* read() override {
        return nullptr;
    };
    
    uint8_t readByte() override {
        if(pos<buffer.length()) {
            uint8_t by = buffer[pos++];
            if(pos>=buffer.length())
                eofOccured = true;

            return by;
        }
        else
            return 0;
    };

    size_t available() override {
        return buffer.length()-pos;
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
    IECOstream* m_os;

    C64LinedWriter(IECOstream* os) : LinedWriter(os), m_os(os) { 
    };

    bool print(std::string line) {
        // line is utf-8, convert to petscii
        Debug_printv("to print:%s",line.c_str());
        StringReader* a = new StringReader(line); // TODO create string reader!

        std::string converted;
        while(!a->eof()) {
            U8Char codePoint(a);
            converted+=codePoint.toPetscii();
        }
        MBufferConst buffer(converted);
        Debug_printv("Converted line:%s", converted.c_str());
        return write(&buffer);        
    }

    bool printLn(std::string line) {
        return print(line+delimiter); 
    }

    bool writeLastByte(uint8_t byteToWrite) 
    {
        m_os->writeLast(&byteToWrite, 1);
        return true;
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

#endif /* MEATFILESYSTEM_WRAPPERS_C64_READER_WRITER */
