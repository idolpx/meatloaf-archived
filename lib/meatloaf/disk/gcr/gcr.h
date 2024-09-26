/*
    gcr.h - Group Code Recording helper functions

    (C) 2001-05 Markus Brenner <markus(at)brenner(dot)de>
    	and Pete Rittwage <peter(at)rittwage(dot)com>
        based on code by Andreas Boose

    V 0.33   improved sector extraction, added find_track_cycle() function
    V 0.34   added MAX_SYNC_OFFSET constant, approximated to 800 GCR bytes
    V 0.35   modified find_track_cycle() interface
    V 0.36   added bad GCR code detection
    V 0.36a  added find_sector_gap(), find_sector0(), extract_GCR_track()
    V 0.36d  Untold number of additions and consequent bugfixes. (pjr)

*/

#ifndef _GCR_
#define _GCR_

#include <inttypes.h>
//#include "integer.h"	/* Basic integer types */

//#define uint8_t unsigned char
//#define DWORD unsigned int
#define MAX_TRACKS_1541 42
#define MAX_TRACKS_1571 (MAX_TRACKS_1541 * 2)
#define MAX_HALFTRACKS_1541 (MAX_TRACKS_1541 * 2)
#define MAX_HALFTRACKS_1571 (MAX_TRACKS_1571 * 2)

/* D64 constants */
#define BLOCKSONDISK (17*21 + 7*19 + 6*18 + 5*17)
#define BLOCKSEXTRA (5*17 + 2*17)
#define MAXBLOCKSONDISK (BLOCKSONDISK+BLOCKSEXTRA)
#define MAX_TRACK_D64 42

/* G64 constants (only needed for current VICE support */
#define G64_TRACK_MAXLEN 7928
#define G64_TRACK_LENGTH (G64_TRACK_MAXLEN+2)

/* NIB format constants */
#define NIB_TRACK_LENGTH 0x2000
#define NIB_HEADER_SIZE 0xFF

/*
    number of GCR bytes until NO SYNC error
    timer counts down from $d000 to $8000 (20480 cycles)
    until timeout when waiting for a SYNC signal
    This is approx. 20.48 ms, which is approx 1/10th disk revolution
    8000 GCR bytes / 10 = 800 bytes
*/
//#define MAX_SYNC_OFFSET 800
/* this was too small for Lode Runner original (805), so increase to 820 */
//#define MAX_SYNC_OFFSET 820
#define MAX_SYNC_OFFSET 0x1500

#define SIGNIFICANT_GAPLEN_DIFF 0x20

#define GCR_BLOCK_HEADER_LEN 24
#define GCR_BLOCK_DATA_LEN   337
#define GCR_BLOCK_LEN (GCR_BLOCK_HEADER_LEN + GCR_BLOCK_DATA_LEN)

/* To calculate the bytes per rotation:

            			4,000,000 * 60
   b/minute = ------------------------------------------------ = x  bytes/minute
             			speed_zone_divisor * 8bits

4,000,000 is the base clock frequency divided by 4.
8 is the number of bits per byte.
60 gets us to a minute of data, which we can then divide by RPM to
get our numbers.

speed zone divisors are 13, 14, 15, 16 for densities 3, 2, 1, 0 respectively
*/

#define DENSITY3 2307692.308 // bytes per minute
#define DENSITY2 2142857.143
#define DENSITY1 2000000.000
#define DENSITY0 1875000.000

/* Some disks have much less data than we normally expect to be able to write at a given density.
	It's like short tracks, but it's a mastering issue not a protection.
    This keeps us from getting errors in the track cycle detection */
#define CAP_MIN_ALLOWANCE 150

/* minimum amount of good sequential GCR for formatted track */
#define GCR_MIN_FORMATTED 64	// chessmaster track 29 is shortest so far

/* Disk Controller error codes */
#define SECTOR_OK           0x01
#define HEADER_NOT_FOUND    0x02
#define SYNC_NOT_FOUND      0x03
#define DATA_NOT_FOUND      0x04
#define BAD_DATA_CHECKSUM   0x05
#define BAD_GCR_CODE        0x06
#define VERIFY_ERROR        0x07
#define WRITE_PROTECTED     0x08
#define BAD_HEADER_CHECKSUM 0x09
#define ID_MISMATCH         0x0b
#define DISK_NOT_INSERTED   0x0f

#define BM_MATCH       0x10
#define BM_NO_CYCLE	   0x20
#define BM_NO_SYNC     0x40
#define BM_FF_TRACK    0x80

#define ALIGN_NONE			0
#define ALIGN_GAP			1
#define ALIGN_SEC0			2
#define ALIGN_LONGSYNC		3
#define ALIGN_WEAK			4
#define ALIGN_VMAX			5
#define ALIGN_AUTOGAP		6

#define GCR_MASK_BAD_FIRST 0
#define GCR_MASK_BAD_LAST 1

/* global variables */
extern char sector_map_1541[];
extern uint8_t speed_map_1541[];
extern int capacity[];
extern int capacity_min[];
extern int capacity_max[];\
extern int gap_match_length;

/* prototypes */
int find_sync(uint8_t ** gcr_pptr, uint8_t * gcr_end);
void convert_4bytes_to_GCR(uint8_t * buffer, uint8_t * ptr);
int convert_4bytes_from_GCR(uint8_t * gcr, uint8_t * plain);
int extract_id(uint8_t * gcr_track, uint8_t * id);
int extract_cosmetic_id(uint8_t * gcr_track, uint8_t * id);
size_t find_track_cycle(uint8_t ** cycle_start, uint8_t ** cycle_stop, int cap_min,
  int cap_max);
size_t find_nondos_track_cycle(uint8_t ** cycle_start, uint8_t ** cycle_stop,
  int cap_min, int cap_max);
uint8_t convert_GCR_sector(uint8_t * gcr_start, uint8_t * gcr_end,
  uint8_t * d64_sector, int track, int sector, uint8_t * id);
void convert_sector_to_GCR(uint8_t * buffer, uint8_t * ptr,
  int track, int sector, uint8_t * diskID, int error, int sectorSize);
uint8_t * find_sector_gap(uint8_t * work_buffer, int tracklen, size_t * p_sectorlen);
uint8_t * find_sector0(uint8_t * work_buffer, int tracklen, size_t * p_sectorlen);
int extract_GCR_track(uint8_t * destination, uint8_t * source, int * align,
  int force_align, size_t cap_min, size_t cap_max);
int replace_bytes(uint8_t * buffer, int length, uint8_t srcbyte, uint8_t dstbyte);
int check_bad_gcr(uint8_t * gcrdata, int length, int fix);
int check_sync_flags(uint8_t * gcrdata, int density, int length);
void bitshift(uint8_t * gcrdata, int length, int bits);
int check_errors(uint8_t * gcrdata, int length, int track, uint8_t * id,
  char * errorstring);
int check_empty(uint8_t * gcrdata, int length, int track, uint8_t * id,
  char * errorstring);
int compare_tracks(uint8_t * track1, uint8_t * track2, int length1, int length2,
  int same_disk, char * outputstring);
int compare_sectors(uint8_t * track1, uint8_t * track2, int length1, int length2,
  uint8_t * id1, uint8_t * id2, int track, char * outputstring);
int strip_runs(uint8_t * buffer, int length, int minrun, uint8_t target);
int reduce_runs(uint8_t * buffer, int length, int length_max, int minrun,
  uint8_t target);
int is_bad_gcr(uint8_t * gcrdata, size_t length, size_t pos);
int check_formatted(uint8_t * gcrdata);
int check_valid_data(uint8_t * data, int length);

#endif
