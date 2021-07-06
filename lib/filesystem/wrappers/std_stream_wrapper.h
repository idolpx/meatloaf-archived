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
        if (!this->srcStream->isOpen()) {
            return std::char_traits<char>::eof();
        }
        else if (this->gptr() == this->egptr()) {
            // the next statement assumes "size" characters were produced (if
            // no more characters are available, size == 0.
            auto buffer = reader->read();

            Debug_printv("MeatIBuff underflow, read bytes=%d", buffer->length());

            this->setg(buffer->getBuffer(), buffer->getBuffer(), buffer->getBuffer() + buffer->length());
        }
        // eback = beginning of get area
        // gptr = current character (get pointer)
        // egptr = one past end of get area
        return this->gptr() == this->egptr()
             ? std::char_traits<char>::eof()
             : std::char_traits<char>::to_int_type(*this->gptr());
    };

    void close() {
        srcStream->close();
    }
};




/********************************************************
 * output stream wrapper
 ********************************************************/

class MeatOBuff : public std::streambuf {
    std::shared_ptr<MOStream> srcStream;
    std::shared_ptr<BufferedWriter> writer;

public:
    MeatOBuff(MFile* mf) {
        srcStream.reset(mf->outputStream());
        writer.reset(new BufferedWriter(srcStream.get()));

        //Debug_printv("MeatOBuff constructor. ostream.isOpen=%d", srcStream->isOpen());
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
    int overflow(int ch  = traits_type::eof()) override
    {
        if (!srcStream->isOpen()) {
            return 0;
        }
        else if ( pbase() == NULL ) {
            // save one char for next overflow:
            //Debug_printv("in wrapper overflow '%c'", ch);

            if ( ch != EOF ) {
                writer->writeByte(ch);
            } else {
                ch = 0;
            }
        } else {
            Debug_printv("**** hey! don't know how to process MeatOBuff with buffer! Nothing written!");
            // char* end = pptr();
            // if ( ch != EOF ) {
            //     *end ++ = ch;
            // }
            // if ( write( pbase(), end - pbase() ) == failed ) {
            //     ch = EOF;
            // } else if ( ch == EOF ) {
            //     ch = 0;
            // }
            // setp( buffer, buffer + bufferSize - 1 );
        }
        return ch;
    };


    /**
     *  @brief  Synchronizes the buffer arrays with the controlled sequences.
     *  @return  -1 on failure.
     *
     *  Each derived class provides its own appropriate behavior,
     *  including the definition of @a failure.
     *  @note  Base class version does nothing, returns zero.
     * 
     * sync: Called on flush, should output any characters in the buffer to the sink. 
     * If you never call setp (so there's no buffer), you're always in sync, and this 
     * can be a no-op. overflow or uflow can call this one, or both can call some 
     * separate function. (About the only difference between sync and uflow is that 
     * uflow will only be called if there is a buffer, and it will never be called 
     * if the buffer is empty. sync will be called if the client code flushes the stream.)
     */
    int sync() { 
//                Debug_printv("in wrapper sync");

        auto smartBuffer = MBuffer(pbase(), pptr()-pbase());
        auto result = writer->write(&smartBuffer);

        return (pptr() == pbase() || result != 0) ? 0 : -1;    
    };

    void close() {
        srcStream->close();
    }

};


/********************************************************
 * Urlstream that can be closed
 ********************************************************/

class MUrlIStream : public std::istream {
    MeatIBuff* buff;
public:
    MUrlIStream(MeatIBuff* ibuf): std::istream(ibuf), buff(ibuf) {};
	~MUrlIStream() {
        close();
    };

    virtual void close() {
        buff->close();
    }
};

class MUrlOStream : public std::ostream {
    MeatOBuff* buff;
public:
    MUrlOStream(MeatOBuff* obuf): std::ostream(obuf), buff(obuf) {};
	~MUrlOStream() {
        close();
    };
    
    virtual void close() {
        buff->close();
    }
};


#endif /* MEATFILESYSTEM_WRAPPERS_STD_STREAM_WRAPPER */
