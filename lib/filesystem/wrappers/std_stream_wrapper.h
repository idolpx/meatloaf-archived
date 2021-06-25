#ifndef MEATFILESYSTEM_WRAPPERS_STD_STREAM_WRAPPER
#define MEATFILESYSTEM_WRAPPERS_STD_STREAM_WRAPPER

#include "wrappers/buffered_io.h"

// https://newbedev.com/how-to-write-custom-input-stream-in-c

/********************************************************
 * Input stream wrapper
 ********************************************************/

class MeatIBuff : public std::streambuf {
    std::shared_ptr<MIStream> srcStream;
    std::shared_ptr<BufferedReader> reader;

public:
    MeatIBuff(MFile* mf) {
        srcStream.reset(mf->inputStream());
        reader.reset(new BufferedReader(srcStream.get()));
    }

    /**
     *  @brief  Fetches more data from the controlled sequence.
     *  @return  The first character from the <em>pending sequence</em>.
     *
     *  Informally, this function is called when the input buffer is
     *  exhausted (or does not exist, as buffering need not actually be
     *  done).  If a buffer exists, it is @a refilled.  In either case, the
     *  next available character is returned, or @c traits::eof() to
     *  indicate a null pending sequence.
     *
     *  For a formal definition of the pending sequence, see a good text
     *  such as Langer & Kreft, or [27.5.2.4.3]/7-14.
     *
     *  A functioning input streambuf can be created by overriding only
     *  this function (no buffer area will be used).  For an example, see
     *  https://gcc.gnu.org/onlinedocs/libstdc++/manual/streambufs.html
     *
     *  @note  Base class version does nothing, returns eof().
     */

    int underflow() override {
        if (this->gptr() == this->egptr()) {
            // the next statement assumes "size" characters were produced (if
            // no more characters are available, size == 0.
            auto buffer = reader->read();

            this->setg(buffer->getBuffer(), buffer->getBuffer(), buffer->getBuffer() + buffer->length());
        }
        return this->gptr() == this->egptr()
             ? std::char_traits<char>::eof()
             : std::char_traits<char>::to_int_type(*this->gptr());
    };
};




/********************************************************
 * output stream wrapper
 ********************************************************/

class MeatOBuff : public std::streambuf {
    std::shared_ptr<MOStream> srcStream;
    std::shared_ptr<BufferedWriter> writer;

    MeatOBuff(MFile* mf) {
        srcStream.reset(mf->outputStream());
        writer.reset(new BufferedWriter(srcStream.get()));
    }

      /**
       *  @brief  Consumes data from the buffer; writes to the
       *          controlled sequence.
       *  @param  __c  An additional character to consume.
       *  @return  eof() to indicate failure, something else (usually
       *           @a __c, or not_eof())
       *
       *  Informally, this function is called when the output buffer
       *  is full (or does not exist, as buffering need not actually
       *  be done).  If a buffer exists, it is @a consumed, with
       *  <em>some effect</em> on the controlled sequence.
       *  (Typically, the buffer is written out to the sequence
       *  verbatim.)  In either case, the character @a c is also
       *  written out, if @a __c is not @c eof().
       *
       *  For a formal definition of this function, see a good text
       *  such as Langer & Kreft, or [27.5.2.4.5]/3-7.
       *
       *  A functioning output streambuf can be created by overriding only
       *  this function (no buffer area will be used).
       *
       *  @note  Base class version does nothing, returns eof().
      */
    int overflow(int _IsUnused  = traits_type::eof()) override
    {
        return traits_type::eof(); 
    };


    /**
     *  @brief  Synchronizes the buffer arrays with the controlled sequences.
     *  @return  -1 on failure.
     *
     *  Each derived class provides its own appropriate behavior,
     *  including the definition of @a failure.
     *  @note  Base class version does nothing, returns zero.
     */
    int sync() { 
        return 0; 
    };
};



#endif /* MEATFILESYSTEM_WRAPPERS_STD_STREAM_WRAPPER */
