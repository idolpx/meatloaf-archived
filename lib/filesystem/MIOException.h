#ifndef MEATFILE_DEFINES_EXCEPTION_H
#define MEATFILE_DEFINES_EXCEPTION_H

#include <exception>

// truned off by default: https://github.com/platformio/platform-ststm32/issues/402

struct IOException : public std::exception {
   const char * what () const throw () {
      return "IO";
   }
};

struct IllegalStateException : public IOException {
   const char * what () const throw () {
      return "Illegal State";
   }
};

struct FileNotFoundException : public IOException {
   const char * what () const throw () {
      return "Not found";
   }
};


#endif