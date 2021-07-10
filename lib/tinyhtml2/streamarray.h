#ifndef MEATLIB_TINYHTML2_STREAMARRAY
#define MEATLIB_TINYHTML2_STREAMARRAY

#include <iostream>
#include "../filesystem/meat_io.h"

clas rostreamarray {
    size_t iterator = 0;
    Meat::ifstream* istream;

public:
    rostreamarray(std::string url) {
        istream = new Meat::ifstream(url);
        istream->open();
    }

    ~rostreamarray() {
        istream->close();
        delete istream;
    }

    char* operator[size_t i] {
        if(i != iterator) {
            iterator = i;
            istream->seek(i);
        }
        iterator++;
        return istream->get();
    }

    size_t size() {
        return istream->size();
    };
}

#endif /* MEATLIB_TINYHTML2_STREAMARRAY */
