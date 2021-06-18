#ifndef MEATLINK_DEFINES_H
#define MEATLINK_DEFINES_H

#include "../../include/global_defines.h"
#include "FS.h"
#include "meat_io.h"

#define RECORD_SIZE 256

class MeatLink {
    MeatLink(MFile* linkFileName) 
    {
        //m_fileSystem = fileSystem;
        m_fileName = linkFileName;

        if( m_fileSystem->exists(m_fileName) )
        {
            uint8_t buffer[RECORD_SIZE] = { 0 };

            Serial.printf("Getting meatlink [%s]", linkFileName.c_str());
            File f_link = m_fileSystem->open(m_fileName, "r+");
            if(!f_link) {
                size_t readCount = f_link.readBytes(buffer, RECORD_SIZE);
                m_url = std::string(buffer); // is this enoug? Or do I have to set size somehow?
                f_link.close();
            }
        }
    }

    std::string get() 
    {
        return m_url;
    }

    void put(std::string url) 
    {
        File f_linkFile = m_fileSystem->open(m_fileName, "w+");

        if(f_linkFile)
        {
            uint8_t buffer[RECORD_SIZE] = { 0 };

            sprintf( (char *)buffer, "%s", url.c_str() );
            f_linkFile.write(buffer, RECORD_SIZE);

            Serial.printf("Writing meatlink file %s: %s\r\n", m_fileName.c_str(), m_url.c_str());                    

            f_linkFile.close();
        }
    }

    private:
        MFile* m_fileName;
        std::string m_url;
};

#endif