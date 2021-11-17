// .DFI - DreamLoad File Archive
// https://www.lemon64.com/forum/viewtopic.php?t=37415#458552
// 

//
// The image starts with track 1. The maximum possible tracknumber is $ff. Every
// track has a fixed size of 256 sectors or 64kBytes. The sectors start at $00
// and the last sector in a track is $ff. This means the offset for a block at
// track t and sector s is:
//
// offset = (t-1)*$10000 + s*$100
//
// Here's an example dfi header for a v1.0 dfi image:
//
// 000000 00 44 52 45 41 4d 4c 4f 41 44 20 46 49 4c 45 20 |.DREAMLOAD FILE |
// 000010 41 52 43 48 49 56 45 00 00 00 01 00 02 00 00 00 |ARCHIVE.........|
// 000020 01 01 01 10 00 00 00 00 00 00 00 00 00 00 00 00 |................|
// 000030 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 |................|
// 000040 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 |................|
// 000050 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 |................|
// 000060 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | |
// 000070 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | |
// 000080 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | |
// 000090 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | |
// 0000a0 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | |
// 0000b0 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | |
// 0000c0 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | |
// 0000d0 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | |
// 0000e0 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | |
// 0000f0 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | |
//
// The header's size is 256 bytes, that's exactly one sector. The header is
// always the first sector in the image (track 1, sector 0).
//
// $00 - $17 : magic (0x00, dreamload file archive, 0x00)
// $18 - $1b : version (bit 0-15: minor, 16-31: major. $00010000 is v1.0)
// $1c - $1f : tracks in this image (a track has always 256 sectors)
// $20 - $21 : root dir track and sektor
// $22 - $23 : bam track and sector
// $24 - $5f : reserved
// $60 - $ff : comment or notes
//
// The BAM's size is always 8192 bytes. It is one continuous block which begins
// at the offset specified in the header (see bytes $22 and $23). 'continuous'
// means there is no link information in the first 2 bytes of a sector. All 8192
// bytes are BAM information.
//
// The bits in the BAM are in ascending order. This means byte 0, bit #0 is the
// bit for track 1, sector 0:
//
// offset bit track sector
// 0 0 1 0
// 0 1 1 1
// ...
// 1 0 1 8
// ...
//
// A directory consists of a header and the directory sectors. It's very similar
// to the 1541, but the DFI directory header has less info than the 1541
// counterpart.
//
// Please do not assume that the directory sectors follow the directory header
// immediately. All sectors for a directory are allocated like sectors for a
// normal file and can be scattered over the whole image if neccessary.
//
// A directory header:
// $00 - $01 : first directory block, track and sector
// $02 : format typ 'M'
// $03 - $8f : reserved, should be 0
// $90 - $9f : directory name
// $a0 - $a1 : shift spaces ($a0)
// $a2 - $a6 : directory id
// $a7 - $aa : shift spaces ($a0)
// $ab - $ac : root directory track and sector
// $ad - $ae : parent directory track and sector
// $af - $ff : reserved, should be 0
//
// The image format was designed with subdirectories in mind, but I did not find
// the time yet to implement this in the tools. The idea is that a directory
// entry with type $86 points to the directory header of the subdir (the $86
// comes from the CMD HD). Every directory, even the root dir, has the link to
// the root directory header at offset $ab-$ac. All non-root directories have a
// link to the parent directory at offset $ad-$ae. The root directory has 0/0
// there.
//
// For some example sources please download the patched cbmconvert
// sources "cbmconvert-2.1.2_dfi.tar.gz" from the retrohackers forum:
// http://retrohackers.org/forum/viewtopic.php?p=434#434
//
