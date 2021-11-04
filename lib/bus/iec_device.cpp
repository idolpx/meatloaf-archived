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
		else if (m_iec_data.command == IEC::IEC_SECOND) // data channel opened
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


