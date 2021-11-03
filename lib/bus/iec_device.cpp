// Meatloaf - A Commodore 64/128 multi-device emulator
// https://github.com/idolpx/meatloaf
// Copyright(C) 2020 James Johnston
//
// Meatloaf is free software : you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Meatloaf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Meatloaf. If not, see <http://www.gnu.org/licenses/>.

//#include "../../include/global_defines.h"
//#include "debug.h"

#include "iec_device.h"
#include "iec.h"
#include "wrappers/iec_buffer.h"

using namespace CBM;
using namespace Protocol;

namespace
{
	// Buffer for incoming and outgoing serial bytes and other stuff.
	char serCmdIOBuf[MAX_BYTES_PER_REQUEST];

} // unnamed namespace

iecDevice::iecDevice(IEC &iec)
	: m_iec(iec),
	m_iec_data(*reinterpret_cast<IEC::Data *>(&serCmdIOBuf[sizeof(serCmdIOBuf) / 2])),
	m_device(0)
{
	reset();
} // ctor

bool iecDevice::begin()
{
	return true;
}

void iecDevice::reset(void)
{
	//m_device.reset();
} // reset



uint8_t iecDevice::service(void)
{
	iecDevice::DeviceState r = DEVICE_IDLE;

	//#ifdef HAS_RESET_LINE
	//	if(m_iec.checkRESET()) {
	//		// IEC reset line is in reset device state
	//		reset();
	//
	//
	//		return IEC::BUS_RESET;
	//	}
	//#endif
	// Wait for it to get out of reset.
	//while (m_iec.checkRESET())
	//{
	//	Debug_println("BUS_RESET");
	//}

	//	noInterrupts();
	IEC::BusState bus_state = m_iec.service(m_iec_data);
	//	interrupts();


	if (bus_state == IEC::BUS_ERROR)
	{
		reset();
		bus_state = IEC::BUS_IDLE;
	}

	// Did anything happen from the controller side?
	else if (bus_state not_eq IEC::BUS_IDLE)
	{
		Debug_printf("DEVICE: [%d] ", m_iec_data.device);

		if (m_iec_data.command == IEC::IEC_OPEN)
		{
			Debug_printf("OPEN CHANNEL %d\r\n", m_iec_data.channel);
			if (m_iec_data.channel == 0)
				Debug_printf("LOAD \"%s\",%d\r\n", m_iec_data.content.c_str(), m_iec_data.device);
			else if (m_iec_data.channel == 1)
				Debug_printf("SAVE \"%s\",%d\r\n", m_iec_data.content.c_str(), m_iec_data.device);
			else {
				Debug_printf("OPEN #,%d,%d,\"%s\"\r\n", m_iec_data.device, m_iec_data.channel, m_iec_data.content.c_str());
			}

			// Open Named Channel
			handleOpen(m_iec_data);

			// Open either file or prg for reading, writing or single line command on the command channel.
			if (bus_state == IEC::BUS_COMMAND)
			{
				// Process a command
				Debug_printv("[Process a command]");
				handleListenCommand(m_iec_data);
			}
			else if (bus_state == IEC::BUS_LISTEN)
			{
				// Receive data
				Debug_printv("[Receive data]");
				handleListenData();	
			}
		}
		else if (m_iec_data.command == IEC::IEC_DATA) // data channel opened
		{
			Debug_printf("DATA CHANNEL %d\r\n", m_iec_data.channel);
			if (bus_state == IEC::BUS_COMMAND)
			{
				// Process a command
				Debug_printv("[Process a command]");
				handleListenCommand(m_iec_data);
			}
			else if (bus_state == IEC::BUS_LISTEN)
			{
				// Receive data
				Debug_printv("[Receive data]");
				handleListenData();	
			}
			else if (bus_state == IEC::BUS_TALK)
			{
				// Send data
				Debug_printv("[Send data]");
				if (m_iec_data.channel == CMD_CHANNEL)
				{
					handleListenCommand(m_iec_data);		 // This is typically an empty command,
				}

				handleTalk(m_iec_data.channel);
			}
		}
		else if (m_iec_data.command == IEC::IEC_CLOSE)
		{
			Debug_printf("CLOSE CHANNEL %d\r\n", m_iec_data.channel);
			if(m_iec_data.channel > 0)
			{
				handleClose(m_iec_data);
			}			
		}
	}
	//Debug_printv("mode[%d] command[%.2X] channel[%.2X] state[%d]", mode, m_iec_data.command, m_iec_data.channel, m_openState);

	return bus_state;
} // service


void iecDevice::handleListenCommand(IEC::Data &iec_data)
{
	if (m_device.select(iec_data.device))
	{
		Debug_printv("!!!! device changed: unit:%d current url: [%s]", m_device.id(), m_device.url().c_str());
		m_mfile.reset(MFSOwner::File(m_device.url()));
		Debug_printv("m_mfile[%s]", m_mfile->url.c_str());
	}

	size_t channel = iec_data.channel;
	m_openState = O_NOTHING;

	if (iec_data.content.size() == 0 )
	{
		Debug_printv("No command to process");

		if ( iec_data.channel == CMD_CHANNEL )
			m_openState = O_STATUS;
		return;
	}

	// 1. obtain command and fullPath
	auto commandAndPath = parseLine(iec_data.content, channel);
	auto referencedPath = Meat::New<MFile>(commandAndPath.fullPath);

	Debug_printv("command[%s]", commandAndPath.command.c_str());
	if (mstr::startsWith(commandAndPath.command, "$"))
	{
		m_openState = O_DIR;
		Debug_printv("LOAD $");
	}	
	else if (mstr::endsWith(commandAndPath.command, "*"))
	{
		// Find first program in listing
		if (m_device.path().empty())
		{
			// If in LittleFS root then set it to FB64
			// TODO: Load configured autoload program
			m_filename = "/.sys/fb64";
		}
		else if (m_filename.size() == 0)
		{	
			// Find first PRG file in current directory
			std::unique_ptr<MFile> entry(m_mfile->getNextFileInDir());

			while (entry != nullptr && entry->isDirectory())
			{
				Debug_printv("extension: [%s]", entry->extension.c_str());
				entry.reset(m_mfile->getNextFileInDir());
			}
			m_filename = entry->name;
		}
		m_openState = O_FILE;
		Debug_printv("LOAD * [%s]", m_filename.c_str());
	}
	else if (mstr::equals(commandAndPath.command, "@info", false))
	{
		m_openState = O_ML_INFO;
	}
	else if (mstr::equals(commandAndPath.command, "@stat", false))
	{
		m_openState = O_ML_STATUS;
	}
	else if (commandAndPath.command == "mfav") {
		// create a urlfile named like the argument, containing current m_mfile path
		// here we don't want the full path provided by commandAndPath, though
		// the full syntax should be: heart:urlfilename,[filename] - optional name of the file that should be pointed to

		auto favStream = Meat::ofstream(commandAndPath.rawPath+".url"); // put the name from argument here!
		favStream.open();
		if(favStream.is_open()) {
			favStream << m_mfile->url;
		}
		favStream.close();
	}
	else if(!commandAndPath.rawPath.empty())
	{
		// 2. fullPath.extension == "URL" - change dir or load file
		if (mstr::equals(referencedPath->extension, "url", false)) 
		{
			// CD to the path inside the .url file
			referencedPath.reset(getPointed(referencedPath.get()));

			if ( referencedPath->isDirectory() )
			{
				Debug_printv("change dir called for urlfile");
				changeDir(referencedPath->url);
			}
			else if ( referencedPath->exists() )
			{
				prepareFileStream(referencedPath->url);
			}
		}
		// 2. OR if command == "CD" OR fullPath.isDirectory - change directory
		if (mstr::equals(commandAndPath.command, "cd", false) || referencedPath->isDirectory())
		{
			Debug_printv("change dir called by CD command or because of isDirectory");
			changeDir(referencedPath->url);
		}
		// 3. else - stream file
		else if ( referencedPath->exists() )
		{
			// Set File
			prepareFileStream(referencedPath->url);
		}
	}

	dumpState();

	// Clear command string
	m_iec_data.content.clear();
} // handleListenCommand


void iecDevice::handleListenData()
{
	Debug_printv("[%s]", m_device.url().c_str());

	saveFile();
} // handleListenData


void iecDevice::handleTalk(byte chan)
{
	Debug_printv("channel[%d] openState[%d]", chan, m_openState);

	switch (m_openState)
	{
		case O_NOTHING:
			break;

		case O_STATUS:
			// Send status
			sendStatus();
			break;

		case O_FILE:
			// Send file
			sendFile();
			break;

		case O_DIR:
			// Send listing
			sendListing();
			break;

		case O_ML_INFO:
			// Send system information
			sendMeatloafSystemInformation();
			break;

		case O_ML_STATUS:
			// Send virtual device status
			sendMeatloafVirtualDeviceStatus();
			break;
	}

	m_openState = O_NOTHING;
} // handleTalk


void iecDevice::handleOpen(IEC::Data &iec_data)
{
	Debug_printv("OPEN Named Channel (%.2d Device) (%.2d Channel)", iec_data.device, iec_data.channel);
	auto channel = channels[iec_data.command];

	// Are we writing?  Appending?
	channels[iec_data.command].name = iec_data.content;
	channels[iec_data.command].cursor = 0;
	channels[iec_data.command].writing = 0;
} // handleOpen


void iecDevice::handleClose(IEC::Data &iec_data)
{
	Debug_printv("CLOSE Named Channel (%.2d Device) (%.2d Channel)", iec_data.device, iec_data.channel);
	auto channel = channels[iec_data.command];

	// If writing update BAM & Directory
	
	// Remove channel from map

} // handleClose

void iecDevice::dumpState() 
{
	Debug_println("");
	Debug_printv("-------------------------------");
	Debug_printv("URL: [%s]", m_mfile->url.c_str());
    Debug_printv("streamPath: [%s]", m_mfile->streamFile->url.c_str());
    Debug_printv("pathInStream: [%s]", m_mfile->pathInStream.c_str());
	Debug_printv("Scheme: [%s]", m_mfile->scheme.c_str());
	Debug_printv("Username: [%s]", m_mfile->user.c_str());
	Debug_printv("Password: [%s]", m_mfile->pass.c_str());
	Debug_printv("Host: [%s]", m_mfile->host.c_str());
	Debug_printv("Port: [%s]", m_mfile->port.c_str());
	Debug_printv("Path: [%s]", m_mfile->path.c_str());
	Debug_printv("File: [%s]", m_mfile->name.c_str());
	Debug_printv("Extension: [%s]", m_mfile->extension.c_str());
	Debug_printv("");
	Debug_printv("m_filename: [%s]", m_filename.c_str());
    Debug_printv("-------------------------------");
} // dumpState