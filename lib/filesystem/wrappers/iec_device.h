#ifndef MEATFILESYSTEM_WRAPPERS_IEC_DEVICE
#define MEATFILESYSTEM_WRAPPERS_IEC_DEVICE

#include "meat_io.h"
#include "string_utils.h"
#include "c64_reader_writer.h"
#include <memory>

class IECDevice {
    // FIRST you have to wrap iec data into our I/OStream API
    // THEN you have to initialize below two with such IOStreams:
    MOstream* iecOstream; // TODO
    MIstream* iecIstream; // TODO

    void signalError(int number, int track=0, int sector=0) {
        // send iec error here like in:
        // void Interface::setDeviceStatus(int number, int track, int sector)


    }

    // This method transfers a file from any path to IEC device, converting it on the fly if it is a TXT file
    bool transferFile(MFile* srcPath) {
        //auto istream = srcPath->inputStream();
        std::unique_ptr<MIstream> istream(srcPath->inputStream());

        // or istream.get() ???
        if(istream == nullptr) {
            signalError(69); // file not found
            return false;
        }

        std::unique_ptr<BufferedReader> reader(new BufferedReader(istream.get()));
        std::unique_ptr<BufferedWriter> writer;

        if(mstr::equals(srcPath->extension, "txt", false)) {
            writer.reset(new C64LinedWriter(iecOstream));
            // we can skip the BOM here, EF BB BF for UTF8
        }
        else {
            writer.reset(new BufferedWriter(iecOstream));
        }
        while(reader->available()>1 && !reader->eof()) {
            bool result = writer->writeByte(reader->readByte());
            if(!result) {
                signalError(72); // write error
                return false;
            }
        }
        if(reader->available() > 1) {
            // this is eof, not the last char!!!
            signalError(70); // timeout reading
            return false;
        }
        else {
            // now let's do the EOF signaling pause...
            delay(10);
            // and send the last byte;
            writer->writeByte(reader->readByte());
        }
        return true;
    }

    // This metod will save a stream from IEC device to any path, converting it on the fly if it is a text file
    bool receiveFile(MFile* dstPath) {
        std::unique_ptr<MOstream> ostream(dstPath->outputStream());

        // or ostream.get() ???
        if(ostream == nullptr) {
            signalError(80); // error saving!
            return false;
        }

        std::unique_ptr<BufferedReader> reader;
        std::unique_ptr<BufferedWriter> writer(new BufferedWriter(ostream.get()));

        if(mstr::equals(dstPath->extension, "txt", false)) {
            reader.reset(new C64LinedReader(iecIstream));
            // we can also write BOM bytes here, EF BB BF for UTF8
        }
        else {
            reader.reset(new BufferedReader(iecIstream));
        }

        while(!reader->eof()) {
            bool result = writer->write(reader->read());
            if(!result) {
                signalError(72); // write error
                return false;
            }
        }
        if(reader->available() > 0) {
            // this is eof, not the last char!!!
            signalError(70); // timeout
            return false;
        }

    }
};

#endif /* MEATFILESYSTEM_WRAPPERS_IEC_DEVICE */
