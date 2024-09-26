#include "g64.h"

// G64 Utility Functions

bool G64MStream::seekBlock(uint64_t index, uint8_t offset)
{
    uint16_t sectorOffset = 0;
    uint8_t track = 0;

    // Debug_printv("track[%d] sector[%d] offset[%d]", track, sector, offset);

    // Determine actual track & sector from index
    do
    {
        track++;
        uint8_t count = getSectorCount(track);
        if (sectorOffset + count < index)
            sectorOffset += count;
        else
            break;

        // Debug_printv("track[%d] speedZone[%d] secotorsPerTrack[%d] sectorOffset[%d]", track, speedZone(track), count, sectorOffset);
    } while (true);
    uint8_t sector = index - sectorOffset;

    this->block = index;
    this->track = track;
    this->sector = sector;

    // Debug_printv("track[%d] sector[%d] speedZone[%d] sectorOffset[%d]", track, sector, speedZone(track), sectorOffset);

    return containerStream->seek((index * block_size) + offset);
}

bool G64MStream::seekSector(uint8_t track, uint8_t sector, uint8_t offset)
{
    uint16_t sectorOffset = 0;

    //Debug_printv("track[%d] sector[%d] offset[%d]", track, sector, offset);

    // Is this a valid track?
    uint16_t c = partitions[partition].block_allocation_map.size() - 1;
    uint8_t start_track = partitions[partition].block_allocation_map[0].start_track;
    uint8_t end_track = partitions[partition].block_allocation_map[c].end_track;
    if (track < start_track || track > end_track)
    {
        Debug_printv("Invalid Track: track[%d] start_track[%d] end_track[%d]", track, start_track, end_track);
        return false;
    }

    // Is this a valid sector?
    c = getSectorCount(track);
    if (sector > c)
    {
        Debug_printv("Invalid Sector: sector[%d] sectorsPerTrack[%d]", sector, c);
        return false;
    }

    track--;
    for (uint8_t index = 0; index < track; ++index)
    {
        sectorOffset += getSectorCount(index + 1);
        //Debug_printv("track[%d] speedZone[%d] secotorsPerTrack[%d] sectorOffset[%d]", (index + 1), speedZone(index), getSectorCount(index + 1), sectorOffset);
    }
    track++;
    sectorOffset += sector;

    this->block = sectorOffset;
    this->track = track;
    this->sector = sector;

    //Debug_printv("track[%d] sector[%d] speedZone[%d] sectorOffset[%d]", track, sector, speedZone(track), sectorOffset);

    return containerStream->seek((sectorOffset * block_size) + offset);
}


uint16_t G64MStream::readContainer(uint8_t *buf, uint16_t size)
{
    return containerStream->read(buf, size);
}
