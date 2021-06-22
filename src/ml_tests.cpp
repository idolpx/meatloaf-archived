#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#endif
#include <WiFiClient.h>

#include <string>

#include "ml_tests.h"
#include "meat_io.h"
#include "iec_host.h"
#include "../../include/global_defines.h"
#include "../../include/make_unique.h"
#include "wrappers/buffered_io.h"
#include "wrappers/line_reader_writer.h"



void testHeader(std::string testName) {
    Serial.println("\n\n******************************");
    Serial.printf("* TESTING: %s\n", testName.c_str());
    Serial.println("******************************\n");
}

void testDiscoverDevices()
{
    size_t d = 4;
    iecHost iec;
    testHeader("Query Bus for Devices");
    for(d; d<31; d++)
        iec.deviceExists(d);
}

void testReader(MFile* readeTest) {
    // /* Test Line reader */
    testHeader("Buffered line reader");

    Serial.printf("* Trying to read file:%s\n", readeTest->url.c_str());

    std::shared_ptr<MIstream> readerStream(readeTest->inputStream());

    if(readerStream->isOpen()) {
        LinedReader reader(readerStream.get());

        if(reader.eof()) {
            Serial.printf("Reader returned EOF! :(");
        }

        while(!reader.eof()) {
            std::string line = reader.readLn();
            Serial.printf("%s\n",line.c_str());
        };
    }
    else {
        Serial.printf("*** ERROR: stream could not be opened!");
    }
}

void dumpFileProperties(MFile* testMFile) {
    Serial.println("\n== File properties ==");
    Serial.printf("Url: %s, isDir = %d\n", testMFile->url.c_str(), testMFile->isDirectory());
    Serial.printf("Scheme: [%s]\n", testMFile->scheme.c_str());
    Serial.printf("Username: [%s]\n", testMFile->user.c_str());
    Serial.printf("Password: [%s]\n", testMFile->pass.c_str());
    Serial.printf("Host: [%s]\n", testMFile->host.c_str());
    Serial.printf("Port: [%s]\n", testMFile->port.c_str());    
    Serial.printf("Path: [%s]\n", testMFile->path.c_str());
    Serial.printf("stream src: [%s]\n", testMFile->streamPath.c_str());
    Serial.printf("path in stream: [%s]\n", testMFile->pathInStream.c_str());
    Serial.printf("File: [%s]\n", testMFile->name.c_str());
    Serial.printf("Extension: [%s]\n", testMFile->extension.c_str());
    Serial.printf("Size: [%d]\n", testMFile->size());
    Serial.printf("-------------------------------\n");
}

void testDirectory(MFile* dir, bool verbose=false) {
    testHeader("A directory");

    Serial.printf("* Listing %s\n", dir->url.c_str());
    std::unique_ptr<MFile> entry(dir->getNextFileInDir());

    while(entry != nullptr) {
        if(verbose)
            dumpFileProperties(entry.get());
        else
            Serial.printf("'%s'\n", entry->url.c_str());
        entry.reset(dir->getNextFileInDir());
    }
}


void testRecursiveDir(MFile* file, std::string indent) {
    if(file->isDirectory()) {
        std::unique_ptr<MFile> entry(file->getNextFileInDir());

        while(entry != nullptr) {
            if(entry->isDirectory())
            {
                Serial.printf("%s%s <dir>\n", indent.c_str(), entry->name.c_str());
                testRecursiveDir(entry.get(), indent+"   ");
            }
            else
            {
                Serial.printf("%s%s\n", indent.c_str(), entry->name.c_str());                
            }

            entry.reset(file->getNextFileInDir());
        }
    }
}

void testCopy(MFile* srcFile, MFile* dstFile) {
    testHeader("Copy file to destination");

    Serial.printf("FROM:%s\nTO:%s\n", srcFile->url.c_str(), dstFile->url.c_str());

    char exampleText[]="Proletariusze wszystkich krajow, laczcie sie!";

    // test 1 - write some string to a plain file in root dir

    // if(!srcFile->exists()) {
    //     Serial.printf("FSTEST: %s doesn't exist!\n", srcFile->path.c_str());
    //     return;
    // }

    if(dstFile->exists()) {
        bool result = dstFile->remove();
        Serial.printf("FSTEST: %s existed, delete reult: %d\n", dstFile->path.c_str(), result);
    }

    //Serial.println("FSTEST copy: attempt obtain istream");
    std::shared_ptr<MIstream> srcStream(srcFile->inputStream());
    //Serial.println("FSTEST copy: attempt obtain ostream");
    std::shared_ptr<MOstream> dstStream(dstFile->outputStream());

    if(!srcStream->isOpen()) {
        Serial.println("FSTEST: couldn't open a stream for reading");
        return;
    }
    else if(!dstStream->isOpen()) {
        Serial.println("FSTEST: couldn't open a stream for writing");
        return;
    }
    else {
        auto br = std::make_unique<BufferedReader>(srcStream.get());
        auto bw = std::make_unique<BufferedWriter>(dstStream.get());

        do {
            auto buffer = br->read();
            
            if(buffer->length() != 0) {
                Serial.printf("FSTEST: Bytes read into buffred reader: %d\n",buffer->length());
                int written = bw->write(buffer);
                Serial.printf("FSTEST: Bytes written into buffred writer: %d\n",written);

            }
        } while (!br->eof());
    }

    srcStream->close(); // not required, closes automagically
    dstStream->close(); // nor required, closes automagically
}

void dumpParts(std::vector<std::string> v) {
    for(auto i = v.begin(); i < v.end(); i++)
        Serial.printf("%s::",(*i).c_str());
}

void testStringFunctions() {
    testHeader("String functions");
    Serial.printf("pa == %s\n", mstr::drop("dupa",2).c_str());
    Serial.printf("du == %s\n", mstr::dropLast("dupa",2).c_str());
}

void testPaths(MFile* testFile, std::string subDir) {
    testHeader("Path ops");
    //std::shared_ptr<MFile> testFile(MFSOwner::File("http://somneserver.com/utilities/disk tools/cie.dnp/subdir/CIE+SERIAL"));
    dumpFileProperties(testFile);

    Serial.printf("We are in: %s\n",testFile->url.c_str());

    std::shared_ptr<MFile> inDnp(testFile->cd("/"+subDir));
    Serial.printf("- cd /%s = '%s'\n", subDir.c_str(), inDnp->url.c_str());

    std::shared_ptr<MFile> inFlash(testFile->cd("//"+subDir));
    Serial.printf("- cd //%s = '%s'\n", subDir.c_str(), inFlash->url.c_str());

    std::shared_ptr<MFile> parallel(testFile->cd("../"+subDir));
    Serial.printf("- cd ../%s = '%s'\n", subDir.c_str(), parallel->url.c_str());

    std::shared_ptr<MFile> inCie(testFile->cd(subDir));
    Serial.printf("- cd %s = '%s'\n", subDir.c_str(), inCie->url.c_str());
}

void testIsDirectory() {
    std::shared_ptr<MFile> testDir(MFSOwner::File("/NOTADIR/"));
    Debug_printv("dir [%s] exists [%d]",testDir->url.c_str(), testDir->isDirectory());
    testDir.reset(MFSOwner::File("/.sys"));
    Debug_printv("dir [%s] exists [%d]",testDir->url.c_str(), testDir->isDirectory());
    testDir.reset(MFSOwner::File("/.sys/"));
    Debug_printv("dir [%s] exists [%d]",testDir->url.c_str(), testDir->isDirectory());
}

void testUrlParser() {
    // Local File System
    testDirectory(MFSOwner::File("CCGMS"), true);
    testDirectory(MFSOwner::File("GAMES"), true);
    testDirectory(MFSOwner::File("/GAMES"), true);

    // URL
    testDirectory(MFSOwner::File("CS:/"), true);
    testDirectory(MFSOwner::File("CS://"), true);
    testDirectory(MFSOwner::File("HTTP://GOOGLE.COM"), true);
    testDirectory(MFSOwner::File("ML://C64.MEATLOAF.CC"), true);

    // Network Protocol
    testDirectory(MFSOwner::File("WS://GS.MEATLOAF.CC"), true);

    // Special File Type
    testDirectory(MFSOwner::File("C64.ML.CC.URL"), true);
}

void testCD() {
    std::shared_ptr<MFile> testDir(MFSOwner::File(""));
    // Debug_printv("dir [%s]",testDir->url.c_str());
    // testDir.reset(testDir->cd("_"));
    // Debug_printv("cd(_) dir [%s]",testDir->url.c_str());
    // testDir.reset(testDir->cd(".."));
    // Debug_printv("cd(..) dir [%s]",testDir->url.c_str());
    // testDir.reset(testDir->cd("_"));
    // Debug_printv("cd(_) dir [%s]",testDir->url.c_str());

    Serial.println("A chain of CDs");
    // make a folder called GAMES on root of flash
    //testDir.reset(MFSOwner::File(""));
    Serial.printf("I'm in %s, changing to GAMES\n", testDir->url.c_str());
    // then on the 64   LOAD"CD:GAMES",8
    testDir.reset(testDir->cd("GAMES"));
    Serial.printf("I'm in %s, changing to _\n", testDir->url.c_str());
    // then LOAD"CD_",8
    testDir.reset(testDir->cd("_"));
    Serial.printf("I'm in %s, changing to GAMES\n", testDir->url.c_str());
    // then LOAD"CD:GAMES",8
    testDir.reset(testDir->cd("GAMES"));
    Serial.printf("I'm in %s, changing to _\n", testDir->url.c_str());
    // then LOAD"CD_",8
    testDir.reset(testDir->cd("_"));
    Serial.printf("I'm in %s, changing to GAMES\n", testDir->url.c_str());
    // then LOAD"CD:GAMES",8
    testDir.reset(testDir->cd("GAMES"));
    Serial.printf("I'm in %s, changing to _\n", testDir->url.c_str());
    // then LOAD"CD_",8
    testDir.reset(testDir->cd("_"));
    Serial.printf("I'm in %s, changing to GAMES\n", testDir->url.c_str());
    // then LOAD"CD:GAMES",8
    testDir.reset(testDir->cd("GAMES"));
    Serial.printf("I'm in %s\n", testDir->url.c_str());
}

void httpStream(char *url)
{
    bool success = true;
    size_t i = 0;
    size_t b_len = 1;
	uint8_t b[b_len];
    Debug_printv("Opening '%s'\r\n", url);
    std::shared_ptr<MFile> file(MFSOwner::File(url));

    if (file->exists())
    {
        size_t len = file->size();
        Debug_printv("File exists! size [%d]\r\n", len);

        std::shared_ptr<MIstream> stream(file->inputStream());

		for(i=0;i < len; i++)
		{
			success = stream->read(b, b_len);
			if (success)
			{
                Serial.write(b, b_len);
            }
        }
        stream->close();
        Debug_println("");
        Debug_printv("%d of %d bytes sent\r\n", i, len);
    }
    else
    {
        Debug_printv("File does not exist.\r\n");
    }
}

void httpGet(char *url)
{
    bool success = true;
    size_t i = 0;
    WiFiClient client;
    HTTPClient http;
    size_t b_len = 1;
	uint8_t b[1];

    http.setUserAgent("some agent");
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.setRedirectLimit(10);

    Serial.printf("[HTTP] begin... [%s]\n", url);
    if (http.begin(client, url)) {  // HTTP

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();
      size_t len = http.getSize();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

            for(i=0;i < len; i++)
            {
                success = client.readBytes(b, b_len);
                if (success)
                {
                    Serial.write(b, b_len);
                }
            }
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      client.stop();
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
}

void runFSTest(std::string dirPath, std::string filePath) {
    Serial.println("**********************************************************************\n\n");
    std::shared_ptr<MFile> testDir(MFSOwner::File(dirPath));
    std::shared_ptr<MFile> testFile(MFSOwner::File(filePath));
    std::shared_ptr<MFile> destFile(MFSOwner::File("/mltestfile"));

    if(!dirPath.empty() && testDir != nullptr) {
        testPaths(testDir.get(),"subDir");
        testDirectory(testDir.get());
        //testRecursiveDir(otherFile.get(),""); // fucks upp littleFS?
    }
    else {
        Serial.println("*** WARNING - directory instance couldn't be created!");
    }

    if(!filePath.empty() && testFile != nullptr) {
        testReader(testFile.get());
        testCopy(testFile.get(), destFile.get());

        Serial.println("\n\n\n*** Please compare file copied to ML aginst the source:\n\n\n");
        testReader(destFile.get());
    }
    else {
        Serial.println("*** WARNING - file instance couldn't be created!");
    }
    Serial.println("**********************************************************************\n\n");
}

void streamTranslationExample(LinedWriter* writer, LinedReader* reader) {
    writer->printLn("This ___ Will look RIGHT on a C64!", &strcodec::petscii);

    auto read = reader->readLn(&strcodec::petscii); // this line read from Commodore will look right heregit !
}

void runTestsSuite() {
    // working, uncomment if you want
    // runFSTest("/.sys", "README"); // TODO - let urlparser drop the last slash!
    // runFSTest("","http://jigsaw.w3.org/HTTP/connection.html");
    //runFSTest("cs:/apps/ski_writer.d64","cs:/apps/ski_writer.d64/EDITOR.HLP");
    
    // not working yet, DO NOT UNCOMMENT!!!
    //runFSTest("http://somneserver.com/utilities/disk tools/cie.dnp/subdir/CIE+SERIAL","");    
    
    //testIsDirectory();
    testUrlParser();
    //testCD();

    //htmlStream("HTTP://MEATLOAF.CC");  // Doesn't work
    //htmlStream("http://MEATLOAF.CC");  // Works!!!
    //htmlStream("http://meatloaf.cc");  // Works!!!

    //testDiscoverDevices();
}
