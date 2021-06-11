#ifndef MEATFILE_STREAM_WRITER_H
#define MEATFILE_STREAM_WRITER_H

#include "buffered_io.h"

class StreamWriter: public BufferedWriter {
public:
    char delimiter = '\n';

    StreamWriter(MOstream* os) : BufferedWriter(os) { 
    };

    bool print(std::string line) {
        MBufferConst buffer(line);
        write(&buffer);        
    }

    bool printLn(std::string line) {
        MBufferConst buffer(line+delimiter);
        write(&buffer);    
    }
};

class StreamReader: public BufferedReader {
    int buffPos;
    std::string lineBuilder;

public:
    char delimiter = '\n';

    StreamReader(MIstream* is) : BufferedReader(is), buffPos(0), lineBuilder("") { 
        read();
    };
    
    StreamReader(const std::function<int(uint8_t* buf, size_t size)>& fn) : BufferedReader(fn), buffPos(0), lineBuilder("") {
        read();
    }

    std::string readLn() {
        if(buffPos==0 && mbuffer.length()==0 && eof())
            return "";

        lineBuilder="";
//Serial.printf("FSTEST: wchodze do realn przy buffPos=%d, buf.len=%d, eof=%d\n", buffPos, mbuffer.length(), eof());
        do {
            // we read through the whole buffer? Let's get more!
            if(buffPos >= mbuffer.length() && !eof()) {
//Serial.printf("FSTEST: doczytywanie bufora");
                buffPos=0;
                read();
            }

            for(; buffPos<mbuffer.length(); buffPos++) {
                if(mbuffer[buffPos]==delimiter) {
                    buffPos++;
                    return lineBuilder;
                } else {
                    //Serial.printf("%d:",buffPos);
                    lineBuilder+=mbuffer[buffPos];
                }
            }
        } while(!eof());

        return lineBuilder;
    };

};

#endif