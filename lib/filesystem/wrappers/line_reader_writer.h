#ifndef MEATFILESYSTEM_WRAPPERS_LINE_READER_WRITER
#define MEATFILESYSTEM_WRAPPERS_LINE_READER_WRITER

#include "buffered_io.h"
#include "../../../include/petscii.h"


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
