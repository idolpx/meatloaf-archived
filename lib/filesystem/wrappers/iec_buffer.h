#ifndef MEATFILESYSTEM_WRAPPERS_IEC_BUFFER
#define MEATFILESYSTEM_WRAPPERS_IEC_BUFFER


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




class oiecstream : public std::ostream {
    oiecbuf* buff;
public:
    oiecstream(IEC* i) : std::ostream(buff) {
        buff = new oiecbuf(i);
    };

    ~oiecstream() {
        delete buff;
    }
};


#endif /* MEATFILESYSTEM_WRAPPERS_IEC_BUFFER */
