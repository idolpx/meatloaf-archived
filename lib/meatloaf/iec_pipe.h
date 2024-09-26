#ifndef MEATLIB_FILESYSTEM_IEC_PIPE
#define MEATLIB_FILESYSTEM_IEC_PIPE

#include "meat_buffer.h"
#include "wrappers/iec_buffer.h"

/*
This is the main IEC named channel abstraction. Meatloaf keeps an array/map of these as long as they're alive
and removes them when channel is closed

- each named channel is connected to one file

- it allows pushing data to/from remote file

- when ATN is received the transfer stops and will be seamlesly resumed on next call to send/receive

- resuming transfer is possible as long as this channel is open

Example usage:

// c64 requests a remote file "http://address/file.prg"
auto newPipe = new iecPipe();
newPipe->establish("http://address/file.prg", std::ios_base::openmode::_S_in, iec);

// store the pipe in your table of channels
.....

// start transfer
newPipe->readFile();
// this method returns only when either happened:
// 1) the file was read wholly so you have to call newPipe->finish() and remove it from your table of channles
// 2) ATN was pulled, so if reading from this channel is later requested again, you fetch it from
//    your table of channels and call: 
//    storedPipe->readFile()
//    you can readFile() as many times as needed, and it will do what you expect (resume from where it was interrupted)
//    as long as it stays open and not EOF.


_S_out == C64 -> IEC -> file
_S_in  == file -> IEC -> C64

*/
typedef enum {
    STATUS_OK = 0,
    STATUS_EOF,
    STATUS_NDA,
    STATUS_BAD,
} pipe_status_t;

class iecPipe {
    Meat::iostream* fileStream = nullptr;
    oiecstream iecStream;
    std::ios_base::openmode mode;
    std::string filename;

public:
    pipe_status_t status = STATUS_OK;

    std::string getFilename() {
        return filename;
    }

    // you call this once, when C64 requested to open the stream
    bool establish(const std::string& fn, std::ios_base::openmode m, systemBus* i) {
        if(fileStream != nullptr)
            return false;

        filename = fn;

        mode = m;
        iecStream.open(i);

        fileStream = new Meat::iostream(filename.c_str(), mode);
    }

    // this stream is disposed of, nothing more will happen with it!
    void finish() {
        if(fileStream != nullptr) {
            iecStream.close(); // will push last byte and send EOI (if hasn't been closed yet)

            delete fileStream;
            fileStream = nullptr;
        }
    }

    bool isActive() {
        if(fileStream != nullptr)
            return fileStream->is_open();
        else
            return false;
    }

    bool seek(std::streampos p) {
        if(mode == std::ios_base::openmode::_S_in) {
            // we are LOADing, so we need to seek within the source file input stream (get buffer):
            fileStream->seekg(p);
            // our iec stream put buffer can potentially contain more bytes ready for read, but since we are
            // seeking, we need to flush them, so they don't get sent to C64 as if they were read from file
            iecStream.flushpbuff(); 
        }
        else if(mode == std::ios_base::openmode::_S_out) {
            // we are SAVE-ing, so, we need to seek within the destination file output stream (put buffer):
            fileStream->seekp(p);
            //iecStream.flushgbuff() - not sure if it will be required
        }

        return false;
    }

    // LOAD - push file bytes (fileStream.get) to C64 (iecStream.write)
    void readFile() {
        // this pipe wasn't itialized or wasn't meant for reading
        if(fileStream == nullptr || mode != std::ios_base::openmode::_S_in) {
            status = STATUS_BAD;
            IEC.senderTimeout();
            return;
        }
        
        // push bytes from file to C64 as long as they're available (eying ATN)
        while(!fileStream->eof() && !fileStream->bad() && !(IEC.flags bitand ATN_PULLED))
        {
            // If this is a socket stream, it might send NDA ("I have no data for you, but keep asking"), so lte's check it!
            (*fileStream).checkNda();
            if((*fileStream).nda()) {
                status = STATUS_NDA;
                IEC.senderTimeout();
            }
            else {
                // let's try to get and send next byte to IEC
                // we will peek next char in case ATN is pulled during attempt to write
                int next_char = fileStream->peek();
             
                if (next_char != EOF) {
                    // we have a char, let's try to send it
                    char ch = static_cast<char>(next_char);
                    iecStream.write(&ch, 1);
                    //Debug_printf("Sending byte to C64: %.2X ", ch);
                }

                if(IEC.flags bitand ATN_PULLED) {
                    // if ATN was pulled, we need to escape from the loop
                    // PRZEM: set some status here?
                    break;
                }
                else {
                    // ATN wasn't pulled, so the byte was succesfully sent, so we can safely flush that byte we peeked from the buffer
                    char nextChar;
                    fileStream->get(nextChar);
                    status = STATUS_OK;
                }
            }
        }

        // the loop exited, we have to check why
        if(IEC.flags bitand ATN_PULLED) {
            // because ATN was pulled!

            // do nothing
        }
        else if(fileStream->eof()) {
            // because we reached EOF on file stream

            status = STATUS_EOF;
            // iecStream.close() call will do the last byte trick to signal EOF on C64 side
            iecStream.close();
        }
        else if(fileStream->bad() || fileStream->fail()) {
            // because something went wrong with file stream

            status = STATUS_BAD;
            IEC.senderTimeout();
        }
    }

    // SAVE - pull C64 bytes (iecStream.get) to remote file (fileStream.put)
    void writeFile() {
        // this pipe wasn't initialized or wasn't meant for writing
        if(fileStream == nullptr || mode != std::ios_base::openmode::_S_out) {
            status = STATUS_BAD;
            IEC.senderTimeout();
            return;
        }
        
        // push bytes from C64 to file as long as they're available (eying ATN)
        while(!fileStream->bad() && !(IEC.flags bitand ATN_PULLED))
        {
            char nextChar;
            //iecStream.get(nextChar); TODO
            if(!iecStream.eof()) {
                fileStream->put(nextChar);
                status = STATUS_OK;
            }
        }
        // so here either the C64 stopped sending bytes to write or the output stream died
        // nothing to see here, move along!
    }
};

#endif /* MEATLIB_FILESYSTEM_IEC_PIPE */
