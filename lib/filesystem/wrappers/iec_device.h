#ifndef MEATFILESYSTEM_WRAPPERS_IEC_DEVICE
#define MEATFILESYSTEM_WRAPPERS_IEC_DEVICE

#include "meat_io.h"
#include "string_utils.h"
#include "c64_reader_writer.h"
#include <memory>

class IECDevice {
    // FIRST you have to wrap iec data into our I/OStream API
    // THEN you have to initialize below two with such IOStreams:
    MOstream* iecOstream; // TODO this will be a special OStream class that does a pause before sending the last character
    MIstream* iecIstream; // TODO

    // This method transfers a file from any path to IEC device, converting it on the fly if it is a TXT file
    bool transferFile(MFile* srcPath) {
        std::unique_ptr<BufferedReader> reader(new BufferedReader(srcPath->inputStream()));
        std::unique_ptr<BufferedWriter> writer;

        if(mstr::equals(srcPath->extension, "txt", false)) {
            writer.reset(new C64LinedWriter(iecOstream));
        }
        else {
            writer.reset(new BufferedWriter(iecOstream));
        }

        while(!reader->eof()) {
            writer->write(reader->read());
        }
    }

    // This metod will save a stream from IEC device to any path, converting it on the fly if it is a text file
    bool receiveFile(MFile* dstPath) {
        std::unique_ptr<BufferedReader> reader;
        std::unique_ptr<BufferedWriter> writer(new BufferedWriter(dstPath->outputStream()));

        if(mstr::equals(dstPath->extension, "txt", false)) {
            reader.reset(new C64LinedReader(iecIstream));
        }
        else {
            reader.reset(new BufferedReader(iecIstream));
        }

        while(!reader->eof()) {
            writer->write(reader->read());
        }
    }
};

#endif /* MEATFILESYSTEM_WRAPPERS_IEC_DEVICE */
