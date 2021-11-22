#include "g64.h"

/********************************************************
 * File implementations
 ********************************************************/

MIStream* G64File::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new G64IStream(containerIstream);
}

// 4-bit value	GCR code[29]
// hex	bin	bin	hex
// 0x0	0000	0.1010	0x0A    a
// 0x1	0001	0.1011	0x0B    b
// 0x2	0010	1.0010	0x12    n  hi 2
// 0x3	0011	1.0011	0x13    m  hi 3
// 0x4	0100	0.1110	0x0E    e
// 0x5	0101	0.1111	0x0F    f
// 0x6	0110	1.0110	0x16    c  hi 6
// 0x7	0111	1.0111	0x17    k  hi 7
// 0x8	1000	0.1001	0x09    9
// 0x9	1001	1.1001	0x19    p  hi 9
// 0xA	1010	1.1010	0x1A    A  hi a
// 0xB	1011	1.1011	0x1B    B  hi b
// 0xC	1100	0.1101	0x0D    d
// 0xD	1101	1.1101	0x1D    D  hi d
// 0xE	1110	1.1110	0x1E    E  hi e
// 0xF	1111	1.0101	0x15    l  hi 5
//
uint8_t G64IStream::decode(uint16_t in)
{
   // give me 5 bits
   switch (in) {
     case 0x0A: return 0x00;
     case 0x0B: return 0x01;
     case 0x12: return 0x02;
     case 0x13: return 0x03;
     case 0x0E: return 0x04;
     case 0x0F: return 0x05;
     case 0x16: return 0x06;
     case 0x17: return 0x07;
     case 0x09: return 0x08;
     case 0x19: return 0x09;
     case 0x1A: return 0x0A;
     case 0x1B: return 0x0B;
     case 0x0D: return 0x0C;
     case 0x1D: return 0x0D;
     case 0x1E: return 0x0E;
     case 0x15: return 0x0F;

     default: Debug_printv(" (NON-GCR-CODE %s%s%s%s%s)",
                        in&0x10 ? "1" : "0",
                        in&0x08 ? "1" : "0",
                        in&0x04 ? "1" : "0",
                        in&0x02 ? "1" : "0",
                        in&0x01 ? "1" : "0"
                        );
        return -1;
        //return 0x00;
   }
}

bool G64IStream::ConvertSector(unsigned track, unsigned sector, unsigned char* data)
{
	unsigned char buffer[SECTOR_LENGTH_WITH_CHECKSUM];
	unsigned char checkSum;
	int index;
	int bitIndex;

	bitIndex = FindSectorHeader(track, sector, 0);
	if (bitIndex < 0)
		return false;

	bitIndex = FindSync(track, bitIndex, (SECTOR_LENGTH_WITH_CHECKSUM * 2) * 8);
	if (bitIndex < 0)
		return false;

	DecodeBlock(track, bitIndex, buffer, SECTOR_LENGTH_WITH_CHECKSUM / 4);

	checkSum = buffer[257];
	for (index = 0; index < SECTOR_LENGTH; ++index)
	{
		data[index] = buffer[index + 1];
		checkSum ^= data[index];
	}

	if (buffer[0] != 0x07)
		return false;			// No data block

	return checkSum == 0;
}

void G64IStream::DecodeBlock(unsigned track, int bitIndex, unsigned char* buf, int num)
{
	int shift, i, j;
	unsigned char gcr[5];
	unsigned char byte;
	unsigned char* offset;
#if defined(EXPERIMENTALZERO)
	unsigned char* end = &tracks[track << 13] + trackLengths[track];
#else
	unsigned char* end = tracks[track] + trackLengths[track];
#endif

	shift = bitIndex & 7;
#if defined(EXPERIMENTALZERO)
	offset = &tracks[track << 13] + (bitIndex >> 3);
#else
	offset = tracks[track] + (bitIndex >> 3);
#endif

	byte = offset[0] << shift;
	for (i = 0; i < num; i++, buf += 4)
	{
		for (j = 0; j < 5; j++)
		{
			offset++;
			if (offset >= end)
#if defined(EXPERIMENTALZERO)
				offset = &tracks[track << 13];
#else
				offset = tracks[track];
#endif
		
			if (shift)
			{
				gcr[j] = byte | ((offset[0] << shift) >> 8);
				byte = offset[0] << shift;
			}
			else 
			{
				gcr[j] = byte;
				byte = offset[0];
			}
		}
		convert_4bytes_from_GCR(gcr, buf);
	}
}

int G64IStream::FindSync(unsigned track, int bitIndex, int maxBits, int* syncStartIndex)
{
	int readShiftRegister = 0;
#if defined(EXPERIMENTALZERO)
	unsigned char byte = tracks[(track << 13) + (bitIndex >> 3)] << (bitIndex & 7);
#else
	unsigned char byte = tracks[track][bitIndex >> 3] << (bitIndex & 7);
#endif
	bool prevBitZero = true;

	while (maxBits--)
	{
		if (byte & 0x80)
		{
			if (syncStartIndex && prevBitZero)
				*syncStartIndex = bitIndex;

			prevBitZero = false;
			readShiftRegister = (readShiftRegister << 1) | 1;
		}
		else
		{
			prevBitZero = true;

			if (~readShiftRegister & 0x3ff)
				readShiftRegister <<= 1;
			else
				return bitIndex;
		}
		if (~bitIndex & 7)
		{
			bitIndex++;
			byte <<= 1;
		}
		else
		{
			bitIndex++;
			if (bitIndex >= int(BitsInTrack(track)))
				bitIndex = 0;
#if defined(EXPERIMENTALZERO)
			byte = tracks[(track << 13)+(bitIndex >> 3)];
#else
			byte = tracks[track][bitIndex >> 3];
#endif
		}
	}
	return -1;
}

int G64IStream::FindSectorHeader(unsigned track, unsigned sector, unsigned char* id)
{
	unsigned char header[10];
	int bitIndex;
	int bitIndexPrev;

	bitIndex = 0;
	bitIndexPrev = -1;
	for (;;)
	{
		bitIndex = FindSync(track, bitIndex, NIB_TRACK_LENGTH * 8);
		if (bitIndexPrev == bitIndex)
			break;
		if (bitIndexPrev < 0)
			bitIndexPrev = bitIndex;
		DecodeBlock(track, bitIndex, header, 2);

		if (header[0] == 0x08 && header[2] == sector)
		{
			if (id)
			{
				id[0] = header[5];
				id[1] = header[4];
			}
			return bitIndex;
		}
	}
	return -1;
}
