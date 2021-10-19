#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#endif
#include <WiFiClient.h>

#include <string>
#include <ArduinoJson.h>

#include "ml_tests.h"
#include "meat_io.h"
#include "iec_host.h"
#include "../../include/global_defines.h"
#include "../../include/make_unique.h"

std::unique_ptr<MFile> m_mfile(MFSOwner::File(""));


void testHeader(std::string testName) {
    Serial.println("\n\n******************************");
    Serial.printf("* TESTING: %s\n", testName.c_str());
    Serial.println("******************************\n");
}

void testDiscoverDevices()
{
    iecHost iec;
    testHeader("Query Bus for Devices");
    for(size_t d=4; d<31; d++)
        iec.deviceExists(d);
}

void testReader(MFile* readeTest) {
    // /* Test Line reader */
    testHeader("C++ line reader");

    Serial.printf("* Trying to read file:%s\n", readeTest->url.c_str());

    auto readerStream = Meat::ifstream(readeTest);
    readerStream.open();

    if(readerStream.is_open()) {
        if(readerStream.eof()) {
            Serial.printf("Reader returned EOF! :(");
        }

        while(!readerStream.eof()) {
            std::string line;

            readerStream >> line;

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

    if(dstFile->exists()) {
        bool result = dstFile->remove();
        Serial.printf("FSTEST: %s existed, delete reult: %d\n", dstFile->path.c_str(), result);
    }

    srcFile->copyTo(dstFile);
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
    Serial.printf("dir [%s] exists [%d]\n",testDir->url.c_str(), testDir->isDirectory());
    testDir.reset(MFSOwner::File("/.sys"));
    Serial.printf("dir [%s] exists [%d]\n",testDir->url.c_str(), testDir->isDirectory());
    testDir.reset(MFSOwner::File("/.sys/"));
    Serial.printf("dir [%s] exists [%d]\n",testDir->url.c_str(), testDir->isDirectory());
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

        std::shared_ptr<MIStream> stream(file->inputStream());

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

void testStdStreamWrapper(MFile* srcFile, MFile* dstFile) {
    testHeader("C++ stream wrappers");

    StaticJsonDocument<512> m_device;
    deserializeJson(m_device, F("{\"id\":0,\"media\":0,\"partition\":0,\"url\":\"http://niceurlman.com\",\"path\":\"/\",\"archive\":\"\",\"image\":\"\"}"));

    if ( dstFile->exists() )
        dstFile->remove();

    Meat::ofstream ostream(dstFile); // dstFile
    ostream.open();
    
    if(ostream.is_open()) {
        Serial.printf("Trying to serialize JSON to %s\n", dstFile->url.c_str());

        auto x = serializeJson(m_device, ostream); 

        Serial.printf("serializeJson returned %d\n", x);

        if(ostream.bad()) {
            Serial.println("WARNING: FILE WRITE FAILED!!!");
        }

        ostream.close();
    }

    //Serial.printf("%s size is %d\n", dstFile->url.c_str(), dstFile->size());

    Serial.printf("Copy %s to %s\n", dstFile->url.c_str(), srcFile->url.c_str());

    if(dstFile->copyTo(srcFile)) {
        Meat::ifstream istream(srcFile);
        istream.open();

        if(istream.is_open()) {
            Serial.printf("Trying to deserialize JSON from %s\n",srcFile->url.c_str());

            deserializeJson(m_device, istream);

            Serial.printf("Got from deserialization: %s\n", ((std::string)m_device["url"]).c_str());
        }
        else
        {
            Serial.printf("Error! The stream for deserialization couldn't be opened!");
        }
    }
    else {
        Serial.println("**** Copying failed *** WHY???");

        Serial.printf("Trying to deserialize JSON from %s\n",dstFile->url.c_str());

        Meat::ifstream newIstream(dstFile); // this is your standard istream!
        newIstream.open();

        deserializeJson(m_device, newIstream);

        Serial.printf("Got from deserialization: %s\n", ((std::string)m_device["url"]).c_str());

    }

}


void testNewCppStreams(std::string name) {
    testHeader("TEST C++ streams");

    Serial.println(" * Read test\n");

    Meat::ifstream istream(name);
    istream.open();
    if(istream.is_open()) {
        std::string line;

        while(!istream.eof()) {
            istream >> line;
            Serial.print(line.c_str());
        }

        istream.close();
    }

    Serial.println(" * Write test\n");

    Meat::ofstream ostream("/intern.txt");

    ostream.open();
    if(ostream.is_open()) {
        ostream << "Arise, ye workers from your slumber,";
        ostream << "Arise, ye prisoners of want.";
        ostream << "For reason in revolt now thunders,";
        ostream << "and at last ends the age of cant!";
        if(ostream.bad())
            Serial.println("WRITING FAILED!!!");

        ostream.close();
    }
}

void runFSTest(std::string dirPath, std::string filePath) {
    //Serial.println("**********************************************************************\n\n");
    // std::shared_ptr<MFile> testDir(MFSOwner::File(dirPath));
    // std::shared_ptr<MFile> testFile(MFSOwner::File(filePath));
    // std::shared_ptr<MFile> destFile(MFSOwner::File("/mltestfile"));

    auto testDir = Meat::New<MFile>(dirPath);
    auto testFile = Meat::New<MFile>(filePath);
    auto destFile = Meat::New<MFile>("/mltestfile");

    testNewCppStreams(filePath);

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

        testStdStreamWrapper(testFile.get(), destFile.get());

        Serial.println("\n\n\n*** Please compare file copied to ML aginst the source:\n\n\n");
        testReader(destFile.get());
    }
    else {
        Serial.println("*** WARNING - file instance couldn't be created!");
    }
    
    Serial.println("**********************************************************************\n\n");
}

void testSmartMFile() {
    testHeader("TEST smart MFile pointers");

    Serial.println("Creating smart MFile from char*");
    auto test = Meat::New<MFile>("cs://some/directory/disk.d64/file.txt");
    Serial.println("Creating smart MFile from MFile*");
    auto test2 = Meat::New<MFile>(test.get());

    auto wrapped = Meat::Wrap<MFile>(test2->getNextFileInDir());

	Serial.printf("Extension of second one: [%s]\n", test2->extension.c_str());
}


void runTestsSuite() {
    // working, uncomment if you want
    //runFSTest("/.sys", "README"); // TODO - let urlparser drop the last slash!
    //runFSTest("","http://jigsaw.w3.org/HTTP/connection.html");
    //runFSTest("cs:/apps/ski_writer.d64","cs:/apps/ski_writer.d64/EDITOR.HLP");
    
    // not working yet, DO NOT UNCOMMENT!!!
    //runFSTest("http://somneserver.com/utilities/disk tools/cie.dnp/subdir/CIE+SERIAL","");    
    
    //testIsDirectory();
    //testUrlParser();
    //testCD();

    //htmlStream("HTTP://MEATLOAF.CC");  // Doesn't work
    //htmlStream("http://MEATLOAF.CC");  // Works!!!
    //htmlStream("http://meatloaf.cc");  // Works!!!

    //testDiscoverDevices();
    // testCDMFile("CCGMS", 0);
    // testCDMFile("CD:GAMES", 0);
    // testCDMFile("CD_", 0);
    // testCDMFile("CD/GAMES", 0);
    // testCDMFile("CD_", 0);
    // testCDMFile("CDGAMES", 0);
    // testCDMFile("CD_", 0);
    // testCDMFile("CDGAMES", 15);
    // testCDMFile("CD_", 0);

    // D64 Test
    // Debug_printv("D64 Test");
    // testDirectory(MFSOwner::File("/games/arcade7.d64"), true);
}
