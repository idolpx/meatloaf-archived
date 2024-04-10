#include "iec_buffer.h"

/********************************************************
 * oiecbuf
 * 
 * A buffer for writing IEC data, handles sending EOI
 ********************************************************/

size_t oiecbuf::easyWrite(bool lastOne) {
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
        Debug_printv("IEC easyWrite writes THE LAST ONE with EOI:%c", *(pptr()-1));

        m_iec->sendEOI(*(pptr()-1));
        setp(data, data+1024);
    }
    else {
        // let's shift the buffer to the last character
        setp(pptr()-1, data+1024);
    }

    return written;
}

int oiecbuf::overflow(int ch) {
    Debug_printv("overflow for iec called");
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

int oiecbuf::sync() { 
    Debug_printv("sync for iec called");
    if(pptr() == pbase()) {
        return 0;
    }
    else {
        auto result = easyWrite(true); 
        return (result != 0) ? 0 : -1;  
    }  
};


/********************************************************
 * oiecstream
 * 
 * Standard C++ stream for writing to IEC
 ********************************************************/

void oiecstream::putUtf8(U8Char* codePoint) {
    //Serial.printf("%c",codePoint->toPetscii());
    //Debug_printv("oiecstream calling put");
    auto c = codePoint->toPetscii();
    put(codePoint->toPetscii());        
}

    // void oiecstream::writeLn(std::string line) {
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
