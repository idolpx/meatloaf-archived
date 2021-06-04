#ifndef MEATFILE_BUFFERED_H
#define MEATFILE_BUFFERED_H

#define RECORD_SIZE 256
#define BUFFER_SIZE 512

#include "meat_io.h"
#include "meat_stream.h"

class MBuffer {
    char* buffer; // will point to buffered reader internal buffer
    int len = 0;

public:
    int length() {
        return len;
    }

    friend class BufferedReader;
    friend class BufferedWriter;
};

class BufferedReader {
    MIstream* istream;
    bool secondHalf = false;
    bool eofOccured = false;

    #if defined(ESP32)
    uint8_t buffer[BUFFER_SIZE] = { 0 };
    #else //defined(ESP8266)
    char buffer[BUFFER_SIZE] = { 0 };
    #endif

    MBuffer mbuffer;

    void refillBuffer() {
        secondHalf = !secondHalf;
        uint8_t* window = (secondHalf) ? (uint8_t*)buffer : (uint8_t*)(buffer+RECORD_SIZE);
        mbuffer.len = istream->read(window, RECORD_SIZE);
        mbuffer.buffer = (char *)window;
        eofOccured = mbuffer.len == 0;
    }

public:
    BufferedReader(MIstream* is) : istream(is) { 
    };
    // read - moze jakas abstrakcja tablicy?
    MBuffer* read() {
        if(!eofOccured)
            refillBuffer();

        return &mbuffer;
    }
    bool eof() {
        return eofOccured;
    }
};

class BufferedWriter {
    MOstream* ostream;
    bool secondHalf = false;

public:
    BufferedWriter(MOstream* os) : ostream(os) { 
    };
    // read - moze jakas abstrakcja tablicy?
    int write(MBuffer* buffer) {
        return ostream->write((uint8_t*)buffer->buffer, buffer->length());
    }
};


#endif