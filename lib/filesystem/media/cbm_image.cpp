#include "cbm_image.h"

// D64 Utility Functions

bool CBMImageStream::seekSector( uint8_t track, uint8_t sector, size_t offset )
{
	uint16_t sectorOffset = 0;

    //Debug_printv("track[%d] sector[%d] offset[%d]", track, sector, offset);

    track--;
	for (uint8_t index = 0; index < track; ++index)
	{
		sectorOffset += sectorsPerTrack[speedZone(index)];
        //Debug_printv("track[%d] speedZone[%d] secotorsPerTrack[%d] sectorOffset[%d]", (index + 1), speedZone(index), sectorsPerTrack[speedZone(index)], sectorOffset);
	}
	sectorOffset += sector;

    this->track = track + 1;
    this->sector = sector;

    return containerStream->seek( (sectorOffset * block_size) + offset );
}

bool CBMImageStream::seekSector( std::vector<uint8_t> trackSectorOffset )
{
    return seekSector(trackSectorOffset[0], trackSectorOffset[1], trackSectorOffset[2]);
}


std::string CBMImageStream::readBlock(uint8_t track, uint8_t sector)
{
    return "";
}

bool CBMImageStream::writeBlock(uint8_t track, uint8_t sector, std::string data)
{
    return true;
}

bool CBMImageStream::allocateBlock( uint8_t track, uint8_t sector)
{
    return true;
}

bool CBMImageStream::deallocateBlock( uint8_t track, uint8_t sector)
{
    return true;
}

bool CBMImageStream::seekEntry( std::string filename )
{
    uint8_t index = 1;
    mstr::rtrimA0(filename);
    mstr::replaceAll(filename, "\\", "/");

    // Read Directory Entries
    if ( filename.size() )
    {
        while ( seekEntry( index ) )
        {
            std::string entryFilename = entry.filename;
            mstr::rtrimA0(entryFilename);
            Debug_printv("track[%d] sector[%d] filename[%s] entry.filename[%.16s]", track, sector, filename.c_str(), entryFilename.c_str());

            // Read Entry From Stream
            if (entry.file_type & 0b00000111 && filename == "*")
            {
                filename == entryFilename;
            }
            
            if ( mstr::startsWith(entryFilename, filename.c_str()) )
            {
                // Move stream pointer to start track/sector
                return true;
            }
            index++;
        }
    }

    entry.next_track = 0;
    entry.next_sector = 0;
    entry.blocks = 0;
    entry.filename[0] = '\0';

    return false;
}

bool CBMImageStream::seekEntry( size_t index )
{
    bool r = false;

    // Calculate Sector offset & Entry offset
    // 8 Entries Per Sector, 32 bytes Per Entry
    index--;
    uint8_t sectorOffset = index / 8;
    uint8_t entryOffset = (index % 8) * 32;

    //Debug_printv("----------");
    //Debug_printv("index[%d] sectorOffset[%d] entryOffset[%d] entry_index[%d]", index, sectorOffset, entryOffset, entry_index);


    if (index == 0 || index != entry_index)
    {
        // Start at first sector of directory
        next_track = 0;
        r = seekSector( directory_list_offset );
        
        // Find sector with requested entry
        do
        {
            if ( next_track )
            {
                //Debug_printv("next_track[%d] next_sector[%d]", entry.next_track, entry.next_sector);
                r = seekSector( entry.next_track, entry.next_sector, 0x00 );
            }

            containerStream->read((uint8_t *)&entry, sizeof(entry));
            next_track = entry.next_track;
            next_sector = entry.next_sector;

            //Debug_printv("sectorOffset[%d] -> track[%d] sector[%d]", sectorOffset, track, sector);
        } while ( sectorOffset-- > 0 );
        r = seekSector( track, sector, entryOffset );
    }
    else
    {
        if ( entryOffset == 0 )
        {
            if ( next_track == 0 )
                return false;

            //Debug_printv("Follow link track[%d] sector[%d] entryOffset[%d]", next_track, next_sector, entryOffset);
            r = seekSector( next_track, next_sector, entryOffset );
        }        
    }

    containerStream->read((uint8_t *)&entry, sizeof(entry));      

    // If we are at the first entry in the sector then get next_track/next_sector
    if ( entryOffset == 0 )
    {
        next_track = entry.next_track;
        next_sector = entry.next_sector;        
    }

    //Debug_printv("r[%d] file_type[%02X] file_name[%.16s]", r, entry.file_type, entry.filename);

    //if ( next_track == 0 && next_sector == 0xFF )
    entry_index = index + 1;    
    if ( entry.file_type == 0x00 )
        return false;
    else
        return true;
}

std::string CBMImageStream::decodeEntry() 
{
    bool hide = false;
    uint8_t file_type = entry.file_type & 0b00000111;
    std::string type = file_type_label[ file_type ];
    if ( file_type == 0 )
        hide = true;

    switch ( entry.file_type & 0b11000000 )
    {
        case 0xC0:			// Bit 6: Locked flag (Set produces "<" locked files)
            type += "<";
            hide = false;
            break;
            
        case 0x00:
            type += "*";	// Bit 7: Closed flag  (Not  set  produces  "*", or "splat" files)
            hide = true;
            break;					
    }

    return type;
}

uint16_t CBMImageStream::blocksFree()
{
    uint16_t free_count = 0;

    for(uint8_t x = 0; x < block_allocation_map.size(); x++)
    {
        uint8_t bam[block_allocation_map[x].byte_count] = { 0 };
        //Debug_printv("start_track[%d] end_track[%d]", block_allocation_map[x].start_track, block_allocation_map[x].end_track);

        seekSector(block_allocation_map[x].track, block_allocation_map[x].sector, block_allocation_map[x].offset);
        for(uint8_t i = block_allocation_map[x].start_track; i <= block_allocation_map[x].end_track; i++)
        {
            containerStream->read((uint8_t *)&bam, sizeof(bam));
            if ( sizeof(bam) > 3 )
            {               
                if ( i != block_allocation_map[x].track )
                {
                    //Debug_printv("x[%d] track[%d] count[%d] size[%d]", x, i, bam[0], sizeof(bam));
                    free_count += bam[0];            
                }
            }
            else
            {
                // D71 tracks 36 - 70 you have to count the 1 bits (0 is allocated)
                uint8_t bit_count = 0;
                bit_count += std::bitset<8>(bam[0]).count();
                bit_count += std::bitset<8>(bam[1]).count();
                bit_count += std::bitset<8>(bam[2]).count();

                //Debug_printv("x[%d] track[%d] count[%d] size[%d] bam0[%d] bam1[%d] bam2[%d] (counting 1 bits)", x, i, bit_count, sizeof(bam), bam[0], bam[1], bam[2]);
                free_count += bit_count;
            }                    
        }
    }

    return free_count;
}

size_t CBMImageStream::readFile(uint8_t* buf, size_t size) {
    size_t bytesRead = 0;

    if ( sector_offset % block_size == 0 )
    {
        // We are at the beginning of the block
        // Read track/sector link
        containerStream->read((uint8_t *)&next_track, 1);
        containerStream->read((uint8_t *)&next_sector, 1);
        sector_offset += 2;
        //Debug_printv("next_track[%d] next_sector[%d] sector_offset[%d]", next_track, next_sector, sector_offset);
    }

    bytesRead += containerStream->read(buf, size);
    sector_offset += bytesRead;
    m_bytesAvailable -= bytesRead;

    if ( sector_offset % block_size == 0 )
    {
        // We are at the end of the block
        // Follow track/sector link to move to next block
        seekSector( next_track, next_sector );
        //Debug_printv("track[%d] sector[%d] sector_offset[%d]", track, sector, sector_offset);
    }

    // if ( !bytesRead )
    // {
    //     sector_offset = 0;
    // }

    return bytesRead;
}