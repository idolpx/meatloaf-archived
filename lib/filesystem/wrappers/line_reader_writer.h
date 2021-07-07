#ifndef MEATFILESYSTEM_WRAPPERS_LINE_READER_WRITER
#define MEATFILESYSTEM_WRAPPERS_LINE_READER_WRITER

#include "buffered_io.h"
#include "../../../include/petscii.h"

// /********************************************************
//  * Simple string codec
//  ********************************************************/

// class StringCodec {
// public:
//     virtual uint8_t toLocal(uint8_t ch)=0;
//     virtual uint8_t toForeign(uint8_t ch)=0;
// };

// class PETSCIICodec: public StringCodec {
//     virtual uint8_t toLocal(uint8_t ch) override {
//         return petscii2ascii(ch);
//     };

//     virtual uint8_t toForeign(uint8_t ch) override {
//         return ascii2petscii(ch);
//     };
// };

// namespace strcodec {
//     static PETSCIICodec petscii;
// };

/********************************************************
 * Stream writer
 ********************************************************/

class LinedWriter: public BufferedWriter {
public:
    char delimiter = '\n';

    LinedWriter(MOStream* os) : BufferedWriter(os) { 
    };

    bool print(std::string line) {
        MBufferConst buffer(line);
        return write(&buffer);        
    }

    bool printLn(std::string line) {
        MBufferConst buffer(line+delimiter);
        return write(&buffer);    
    }

    // bool print(std::string line, StringCodec* codec) {
    //     auto l = line.length();
    //     MBufferConst buffer(line);
    //     MBuffer encoded(l);

    //     for(size_t i=0; i<l; i++) {
    //         encoded[i] = codec->toForeign(buffer[i]);
    //     }

    //     return write(&encoded);        
    // }

    // bool printLn(std::string line, StringCodec* codec) {
    //     return print(line+delimiter, codec);    
    // }
};

/********************************************************
 * Stream reader
 ********************************************************/

class LinedReader: public BufferedReader {
    int buffPos;
    std::string lineBuilder;

public:
    char delimiter = '\n';

    LinedReader(MIStream* is) : BufferedReader(is), buffPos(0), lineBuilder("") { };
    
    LinedReader(const std::function<int(uint8_t* buf, size_t size)>& fn) : BufferedReader(fn), buffPos(0), lineBuilder("") {}

    virtual ~LinedReader() {}

    bool eof() {
        return buffPos >= smartBuffer.length() && BufferedReader::eof();
    }

    std::string readLn() {
        if(buffPos==0 && smartBuffer.length()==0 && BufferedReader::eof()) {
            Debug_printv("EOF!");

            return "";
        }
        lineBuilder="";
        do {
            // we read through the whole buffer? Let's get more!
            if(buffPos >= smartBuffer.length() && !BufferedReader::eof()) {
                buffPos=0;
                read();
            }

            for(; buffPos<smartBuffer.length(); buffPos++) {
                if(smartBuffer[buffPos]==delimiter) {
                    buffPos++;
                    return lineBuilder;
                } else {
                    // if(codec != nullptr)
                    //     lineBuilder+=codec->toLocal(smartBuffer[buffPos]);
                    // else
                        lineBuilder+=smartBuffer[buffPos];
                }
            }
        } while(!eof());

        return lineBuilder;
    };
};

#endif /* MEATFILESYSTEM_WRAPPERS_LINE_READER_WRITER */
