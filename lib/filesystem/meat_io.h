#ifndef MEATLIB_FILESYSTEM_MEAT_IO
#define MEATLIB_FILESYSTEM_MEAT_IO

#include <memory>
#include <Arduino.h>
#include <string>
#include <vector>
#include <fstream>

#include <FS.h>
#include <LittleFS.h>

#include "wrappers/buffered_io.h"
#include "meat_stream.h"
#include "peoples_url_parser.h"
#include "string_utils.h"

/********************************************************
 * Universal file
 ********************************************************/

class MFile : public PeoplesUrlParser {
public:
    MFile() {}; // only for local FS!!!
    MFile(nullptr_t null) : m_isNull(true) {};
    MFile(std::string path);
    MFile(std::string path, std::string name);
    MFile(MFile* path, std::string name);
    MFile* parent(std::string = "");
    MFile* localParent(std::string);
    MFile* root(std::string);
    MFile* localRoot(std::string);
    //std::string name();
    //std::string extension();

    std::string media_root;
    std::string media_header;
    std::string media_id;
    std::string media_image;
    uint16_t media_blocks_free = 0;
    uint16_t media_block_size = 256;
    

    bool operator!=(nullptr_t ptr);

    bool copyTo(MFile* dst) {
        std::unique_ptr<MIStream> istream(this->inputStream());
        std::unique_ptr<MOStream> ostream(dst->outputStream());

        return istream->pipeTo(ostream.get());
    };

    virtual MFile* cd(std::string newDir);
    virtual bool isDirectory() = 0;
    virtual MIStream* inputStream();
    virtual MOStream* outputStream() = 0 ; // has to return OPENED stream
    virtual time_t getLastWrite() = 0 ;
    virtual time_t getCreationTime() = 0 ;
    virtual bool rewindDirectory() = 0 ;
    virtual MFile* getNextFileInDir() = 0 ;
    virtual bool mkDir() = 0 ;
    virtual bool exists() = 0;
    virtual size_t size() = 0;
    virtual bool remove() = 0;
    virtual bool rename(std::string dest) = 0;
    virtual std::string petsciiName() {
        std::string pname = name;
        mstr::toPETSCII(pname);
        return pname;
    }

    virtual ~MFile() {
        //Debug_printv("Deleting: [%s]", this->url.c_str());

    };

    std::string streamPath;
    std::string pathInStream;

protected:
    virtual MIStream* createIStream(MIStream* src) = 0;
    bool m_isNull;
    void fillPaths(std::vector<std::string>::iterator* matchedElement, std::vector<std::string>::iterator* fromStart, std::vector<std::string>::iterator* last);
friend class MFSOwner;
};

/********************************************************
 * Filesystem instance
 * it knows how to create a MFile instance!
 ********************************************************/

class MFileSystem {
public:
    MFileSystem(const char* symbol);
    virtual ~MFileSystem() = 0;
    virtual bool mount() { return true; };
    virtual bool umount() { return true; };
    virtual bool handles(std::string path) = 0;
    virtual MFile* getFile(std::string path) = 0;
    bool isMounted() {
        return m_isMounted;
    }

    static bool byExtension(const char* ext, std::string fileName) {
        return mstr::endsWith(fileName, ext, false);
    }

protected:
    const char* symbol;
    bool m_isMounted;

    friend class MFSOwner;
};


/********************************************************
 * MFile factory
 ********************************************************/

class MFSOwner {
    static std::vector<MFileSystem*> availableFS;

public:
    static MFile* File(std::string name);
    static MFile* File(std::shared_ptr<MFile> file);
    static MFile* File(MFile* file);

    static bool mount(std::string name);
    static bool umount(std::string name);
};

/********************************************************
 * Meat namespace, standard C++ buffers and streams
 ********************************************************/

namespace Meat {
    struct _Unique_mf {
        typedef std::unique_ptr<MFile> _Single_file;
    };

    // Creates a unique_ptr<MFile> for a given url

    /**
    *  @brief  Creates a unique_ptr<MFile> instance froma given url
    *  @param  url  The url to the file.
    *  @return  @c unique_ptr<MFile>
    */
    template<class MFile>
        typename _Unique_mf::_Single_file
        New(std::string url) {
            return std::unique_ptr<MFile>(MFSOwner::File(url));
        }

    /**
    *  @brief  Creates a unique_ptr<MFile> instance froma given url
    *  @param  url  The url to the file.
    *  @return  @c unique_ptr<MFile>
    */
    template<class MFile>
        typename _Unique_mf::_Single_file
        New(char* url) {
            return std::unique_ptr<MFile>(MFSOwner::File(std::string(url)));
        }

    /**
    *  @brief  Creates a unique_ptr<MFile> instance froma given MFile
    *  @param  file  The url to the file.
    *  @return  @c unique_ptr<MFile>
    */
    template<class MFile>
        typename _Unique_mf::_Single_file
        New(MFile* file) {
            return std::unique_ptr<MFile>(MFSOwner::File(file->url));
        }

    /**
    *  @brief  Wraps MFile* into unique_ptr<MFile> so it closes itself as required
    *  @param  file  The url to the file.
    *  @return  @c unique_ptr<MFile>
    */
    template<class MFile>
        typename _Unique_mf::_Single_file
        Wrap(MFile* file) {
            return std::unique_ptr<MFile>(file);
        }

/********************************************************
 * C++ Input MFile buffer
 ********************************************************/

    class imfilebuf : public std::filebuf {
        std::unique_ptr<MIStream> mistream;
        std::unique_ptr<MFile> mfile;
        char data[1024] = { 0 };

    public:
        imfilebuf() {};

        ~imfilebuf() {
            close();
        }

        std::filebuf* open(std::string filename) {
            mfile.reset(MFSOwner::File(filename));
            mistream.reset(mfile->inputStream());
            if(mistream->isOpen())
                return this;
            else
                return nullptr;
        };

        std::filebuf* open(MFile* file) {
            mfile.reset(MFSOwner::File(file->url));
            mistream.reset(mfile->inputStream());
            if(mistream->isOpen())
                return this;
            else
                return nullptr;
        };

        virtual void close() {
            mistream->close();
        }

        bool is_open() const {
            if(mistream == nullptr)
                return false;
            else
                return mistream->isOpen();
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

// // https://newbedev.com/how-to-write-custom-input-stream-in-c


        int underflow() override {
            if (!is_open()) {
                return std::char_traits<char>::eof();
            }
            else if (this->gptr() == this->egptr()) {
                // the next statement assumes "size" characters were produced (if
                // no more characters are available, size == 0.
                //auto buffer = reader->read();

                auto readCount = mistream->read((uint8_t*)data, 1024);

                Debug_printv("imfilebuf underflow, read bytes=%d", readCount);

                this->setg(data, data, data + readCount);
            }
            // eback = beginning of get area
            // gptr = current character (get pointer)
            // egptr = one past end of get area
            return this->gptr() == this->egptr()
                ? std::char_traits<char>::eof()
                : std::char_traits<char>::to_int_type(*this->gptr());
        };

    };

/********************************************************
 * C++ Output MFile buffer
 ********************************************************/

    class omfilebuf : public std::filebuf {
        std::unique_ptr<MOStream> mostream;
        std::unique_ptr<MFile> mfile;
        char data[1024] = { 0 };

    public:
        omfilebuf() {};

        ~omfilebuf() {
            close();
        }

        std::filebuf* open(std::string filename) {
            mfile.reset(MFSOwner::File(filename));
            mostream.reset(mfile->outputStream());
            if(mostream->isOpen())
                return this;
            else
                return nullptr;
        };

        std::filebuf* open(MFile* file) {
            mfile.reset(MFSOwner::File(file->url));
            mostream.reset(mfile->outputStream());
            if(mostream->isOpen())
                return this;
            else
                return nullptr;
        };

        virtual void close() {
            mostream->close();
        }

        bool is_open() const {
            if(mostream == nullptr)
                return false;
            else
                return mostream->isOpen();
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


        // // https://newbedev.com/how-to-write-custom-input-stream-in-c

        int overflow(int ch  = traits_type::eof()) override
        {
            if (!is_open()) {
                return 0;
            }
            else if ( pbase() == NULL ) {
                // save one char for next overflow:
                //Debug_printv("in wrapper overflow '%c'", ch);

                if ( ch != EOF ) {
                    //writer->writeByte(ch); //TODO
                    ch = 6666; // dummy, remove
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

            //auto result = mostream->write(pbase(), pptr()-pbase()); TODO
            int result = 0;

            return (pptr() == pbase() || result != 0) ? 0 : -1;    
        };


    };

/********************************************************
 * C++ Input MFile stream
 ********************************************************/

    class ifstream : public std::istream {
        imfilebuf buff;
        std::string url; 
    public:
        ifstream(std::string u): std::istream(&buff), url(u) {};
        //ifstream(MFile* file) std::istream(&buff), url(file->url) {};

        ~ifstream() {
            close();
        };

        virtual void open() {
            buff.open(url);
        }

        virtual void close() {
            buff.close();
        }

        virtual bool is_open() {
            return buff.is_open();
        }
    };

/********************************************************
 * C++ Output MFile stream
 ********************************************************/

    class ofstream : public std::ostream {
        omfilebuf buff;
        std::string url; 
    public:
        ofstream(std::string u): std::ostream(&buff), url(u) {};
        //ofstream(MFile* file) std::istream(&buff), url(file->url) {};

        ~ofstream() {
            close();
        };

        virtual void open() {
            buff.open(url);
        }

        virtual void close() {
            buff.close();
        }

        virtual bool is_open() {
            return buff.is_open();
        }
    };
}

#endif /* MEATLIB_FILESYSTEM_MEAT_IO */
