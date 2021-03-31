/*********************************************************************
 *
 * file: diskflash.c
 *
 *	This is a disk device driver that operates in the internal
 *	flash of the processor. 
 *
 *	The flash is erasable on 1K boundaries, but there are interactions
 *	within each 4K block that I'm simplifying here by treating the
 *	flash as if it were composed of 4K blocks. Writing a sector
 *	will cause the 4K block containing the sector to be copied into
 *	a track buffer and the affected sector modified in the buffer.
 *	Flushing the buffer erases the 4 1K blocks covered by the buffer,
 *	after which the buffer is written to the flash.
 *
 * revisions:
 *
 *	2013-12-21 rli: liberated from an ARM port and worked over.
 *	  Read-only for now.
 *
 *	2013-12-28 rli: switch to 4K tracks and reserve only 32K for
 *	  the system.
 *
 *********************************************************************/

/***********************
 *
 *	INCLUDES
 *
 ***********************/

#include "bios.h"
#include "modubios.h"
#include "cpureg.h"

/***********************
 *
 *	PROTOTYPES
 *
 ***********************/

void diskflash_cboot( unsigned int drive );
void diskflash_wboot( unsigned int drive );
bios_dph_t *diskflash_select( unsigned char drive, unsigned char logged ); 
unsigned short int diskflash_read_miss( modubios_diskxfer_t *xfer );
unsigned short int diskflash_read_hit( modubios_diskxfer_t *xfer );
unsigned short int diskflash_read( modubios_diskxfer_t *xfer );
unsigned short int diskflash_write( modubios_diskxfer_t *xfer,
  unsigned short int type );
unsigned short int diskflash_flush( unsigned int drive );
void diskflash_eraseblock( unsigned int address );
void diskflash_erasetrack( unsigned int track );
void diskflash_burnsector( unsigned int sector );
void diskflash_burntrack( void );
void diskflash_buffersector( unsigned int track, unsigned int sector );
void diskflash_buffertrack( unsigned int track );

/***********************
 *
 *	CONSTANTS
 *
 ***********************/

/***
 *
 * diskflash_base
 *
 *	Supplies the base address of the memory region holding the disk.
 *
 * revisions:
 *
 *	2013-12-21 rli: flash starts at zero.
 *
 ***/

#define diskflash_base 0x00000000

/***********************
 *
 * 	GLOBALS
 *
 ***********************/

/***
 *
 * diskflash
 *
 *	This structure describes the driver to the modular BIOS.
 *
 * revisions:
 *
 *	2013-05-01 rli: liberated from a4dim and worked over.
 *
 ***/

modubios_disk_t diskflash = {
  0x006d6f72,		/* "rom" */
  0x20131221,
  diskflash_cboot,
  diskflash_wboot,
  diskflash_select,
  diskflash_read,
  diskflash_write,
  diskflash_flush,
  0,
};

/***
 *
 * diskflash_dpb
 *
 *	This disk parameter block describes the disk to the system.
 *	We have 448KB to play with, so we have to use 2KB clusters.
 *
 * revisions:
 *
 *	2013-04-26 rli: reworked for flash image. 2k per track,
 *	  because I can. 
 *
 *	2013-05-01 rli: expand drive to cover the whole 512K, with
 *	  64K reserved for the system.
 *
 *	2013-12-28 rli: 4K per track, 32K reserved for system.
 *
 ***/

bios_dpb_t diskflash_dpb = {
   32,  /* 128-byte sectors per track */
    4,  /* block shift */
   15,  /* block mask */
    1,  /* extent mask */
    0,  /* reserved */
  239,  /* allocation blocks per disk - 1 */
  127,  /* number of directory entries - 1 */
    0,  /* reserved; initial allocation vector in CP/M-80 */
    0,  /* size of check vector */
    8   /* track offset */
};

/***
 *
 * diskflash_alv
 *
 *	This is the allocation vector for the disk. There is
 *	one bit for each block on the disk.
 *
 * revisions:
 *
 *	2010-08-11 rli: renamed.
 *
 *	2013-04-26 rli: adjusted size for rom disk.
 *
 ***/

unsigned char diskflash_alv[ ( 223 / 8 ) + 1 ];

/***
 *
 * diskflash_dph
 *
 *	This is the disk parameter header for the disk. When the
 *	BIOS selects the drive, it receives a pointer to this structure,
 *	which it uses to locate the others.
 *
 *	The modular BIOS contains a scratch block that can be used by
 *	all drives.
 *
 * revisions:
 *
 *	2010-08-11 rli: renaming. don't need a check vector.
 *
 *	2010-08-22 rli: added spare word for alignment.
 *
 *	2010-08-24 rli: retracted spare word.
 *
 *	2013-04-26 rli: don't need to skew sector numbers.
 *
 ***/

extern unsigned char bios_dirbuf[];

bios_dph_t diskflash_dph = {
  0,			/* sector translation table */
  0, 0, 0, 		/* scratch words */
  bios_dirbuf,		/* directory buffer */
  &diskflash_dpb,		/* disk parameter block */
  0,			/* checksum vector */
  diskflash_alv		/* allocation vector */
};

/***
 *
 * diskflash_image
 *
 *	This is the disk image.
 *
 * revisions:
 *
 *	2013-05-01 rli: liberated from a4dim and worked over.
 *
 ***/

unsigned char *diskflash_image = (unsigned char *)diskflash_base;

/***
 *
 * diskflash_trackbuffer
 *
 *	This buffer holds a track of data that is to be written back
 *	to the flash on a flush.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 ***/

struct {
  unsigned int modified;
  unsigned int track;
  unsigned char data[ 4 * 1024 ];
} diskflash_trackbuffer = { 0, };

/*******************
 *
 *	CODE
 *
 *******************/

/***
 *
 * diskflash_cboot
 *
 *	This function is called for a cold boot.
 *
 * revisions:
 *
 *	2013-05-01 rli: liberated from a4dim and worked over.
 *
 * parameters:
 *
 *	- drive: Indicates which drive is being initialized.
 *
 ***/

void diskflash_cboot( unsigned int drive )
{
  return;
}

/***
 *
 * diskflash_wboot
 *
 *	This function is called for a warm boot. 
 *
 * revisions:
 *
 *	2013-05-01 rli: liberated from a4dim and worked over.
 *
 *	2013-12-28 rli: if the track buffer is dirty, flush it.
 *
 * parameters:
 *
 *	- drive: Indicates which drive is being initialized.
 *
 * return value:
 *
 *	none.
 *
 ***/

void diskflash_wboot( unsigned int drive )
{
  if( diskflash_trackbuffer.modified != 0 ) 
    diskflash_flush( drive );
  return;
}

/***
 *
 * diskflash_select
 *
 *	This function is called to select the drive. It returns a
 *	pointer to the drive parameter header, from which the BIOS
 *	locates the rest of the data structures.
 *
 * revisions:
 *
 *	2010-08-11 rli: original version.
 *
 * parameters:
 *
 *	- drive: Indicates which drive is being selected. It is
 *	  zero-based. Since the drive select has been filtered by
 *	  the modular BIOS (in order to find us) and this driver
 *	  supports only one drive, we have no need for this.
 *
 *	- logged: Indicates whether the disk has previously been
 *	  logged in. The least-significant bit is clear if the drive
 *	  has not been logged in. We have no need for this.
 *
 * return value:
 *
 *	- A pointer to the drive parameter header.
 *
 ***/

bios_dph_t *diskflash_select( unsigned char drive, unsigned char logged )
{
  return &diskflash_dph;
}

/***
 *
 * diskflash_flush
 *
 *	This function is called when it's time to flush the buffers.
 *	If the track buffer is dirty, it is written to the flash.
 *
 * revisions:
 *
 *	2013-12-28 rli: flush the track buffer.
 * 
 * parameters:
 *
 *	- drive: Indicates which drive is being flushed.
 *
 * return value:
 *
 *	- 0 is always returned to indicate the flush was successful.
 *
 ***/

unsigned short int diskflash_flush( unsigned int drive )
{

  /* if the track buffer isn't dirty, we don't need to do anything.
   */

  if( diskflash_trackbuffer.modified == 0 ) return 0;

  /* the track buffer is dirty; erase the track in flash and write
   * the buffer to it.
   */

  diskflash_erasetrack( diskflash_trackbuffer.track );
  diskflash_burntrack();

  /* the track buffer is no longer dirty.
   */

  diskflash_trackbuffer.modified = 0;

  return 0;
}

/***
 *
 * diskflash_movesector
 *
 *	This function moves a sector of data from one location to
 *	another.
 *
 *	We can't move 32-bit longwords because the user buffer may
 *	be unaligned.
 *
 * revisions:
 *
 *	2010-08-11 rli: original version.
 *
 * parameters:
 *
 *	- from: Points to the buffer from which data is to be read.
 *
 *	- to: Points to the buffer ot which data is to be written.
 *
 * return value:
 *
 *	none.
 *
 ***/

void diskflash_movesector( unsigned char *from, unsigned char *to )
{
  int count;

  for( count = 0; count < 128; count++ )
    *to++ = *from++;
}

/***
 *
 * diskflash_read_miss
 *
 *	This function is called to read a sector from the disk when
 *	the track involved is not in the buffer.
 *
 * revisions:
 *
 *	2010-08-11 rli: original version.
 *
 *	2013-04-26 rli: update for 16 sectors per track.
 *
 *	2013-12-28 rli: use the dpb instead of a hard-coded
 *	  sectors per track, so I don't have to change it when
 *	  I update the disk format.
 *
 *	  rename to readmiss.
 *
 * parameters:
 *
 *	- xfer: Describes the transfer. The sector number has been
 *	  translated.
 *
 * return value:
 *
 *	- 0 is always returned to indicate that the operation succeeded.
 *
 ***/

unsigned short int diskflash_read_miss( modubios_diskxfer_t *xfer )
{
  unsigned int offset;

  offset = ( ( xfer->track * diskflash_dpb.spt ) + xfer->sector ) * 128;

  diskflash_movesector( 
    &diskflash_image[ offset ], 
    (unsigned char *)xfer->buffer );

  return 0;

}

/***
 *
 * diskflash_read_hit
 *
 *	This function reads a sector from a track that is in the 
 *	buffer; since the track has been modified, the data needs to
 *	come from the buffer rather than the flash.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 * parameters:
 *
 *	- xfer: describes the transfer.
 *
 * return value:
 *
 *	- 0 is always returned to indicate success.
 *
 ***/

unsigned short int diskflash_read_hit( modubios_diskxfer_t *xfer )
{
  unsigned int offset;

  /* we know the track is the one in the buffer, so we only need the
   * sector number to figure out where it lives.
   */

  offset = xfer->sector * 128;

  diskflash_movesector( 
    &diskflash_trackbuffer.data[ offset ], 
    (unsigned char *)xfer->buffer );

  return 0;
}

/***
 *
 * diskflash_read
 *
 *	This function handles a disk read. It determines whether the
 *	track involved is in the track buffer and passes the read along
 *	to the appropriate handler.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 * parameters:
 *
 *	- xfer: Describes the transfer.
 *
 * return value:
 *
 *	see diskflash_read_miss and diskflash_read_hit.
 *
 ***/

unsigned short int diskflash_read( modubios_diskxfer_t *xfer )
{
  if( ( diskflash_trackbuffer.modified != 0 ) &&
    ( diskflash_trackbuffer.track == xfer->track ) ) 
    return diskflash_read_hit( xfer );

  return diskflash_read_miss( xfer );
}

/***
 *
 * diskflash_write
 *
 *	This function is called to write a sector to the disk.
 *
 *	A disk write is performed by loading the affected track into
 *	the buffer and modifying the affected sector. Eventually
 *	this buffer will be flushed back to the flash.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 * parameters:
 *
 *	- xfer: Describes the transfer. The sector number has been
 *	  translated.
 *
 *	- type: Indicates what type of write is being performed. Types
 *	  are:
 *
 *	  - 0: normal write (write to previously allocated block)
 *	  - 1: write to directory sector
 *	  - 2: write to previously unallocated block
 *
 *	  But we don't care about all that.
 *
 * return value:
 *
 *	- 0 is always returned to indicate success.
 *
 ***/

unsigned short int diskflash_write( modubios_diskxfer_t *xfer,
  unsigned short int type )
{

  /* If there's another track in the buffer, it needs to be flushed
   * back to the flash.
   */

  if( ( diskflash_trackbuffer.modified != 0 ) &&
    ( diskflash_trackbuffer.track != xfer->track ) ) {
    diskflash_flush( xfer->drive );
  }

  /* If the buffer remains modified, it already contains the track
   * we're interested in; otherwise, we need to load it.
   */

  if( diskflash_trackbuffer.modified == 0 ) {
    diskflash_buffertrack( xfer->track );
  }

  /* The buffer contains the track of interest. Copy the sector to it.
   */

  diskflash_movesector(
    (unsigned char *)xfer->buffer,
    &diskflash_trackbuffer.data[ xfer->sector << 7 ] );

  /* We've modified the track buffer.
   */

  diskflash_trackbuffer.modified = 1;

  return 0;
}

/***
 *
 * diskflash_eraseblock
 *
 *	This function erases a single block of the flash.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 * parameters:
 *
 *	- address: The address of the first byte in the block that is
 *	  to be erased.
 *
 * return value:
 *
 *	none.
 *
 ***/

void diskflash_eraseblock( unsigned int address )
{

  /* instruct the flash controller to erase the block.
   */

  cpureg_fma_g = address;
  cpureg_fmc_g = cpureg_fmc_erase_c;

  /* wait for the operation to complete.
   */

  while( ( cpureg_fmc_g & cpureg_fmc_erase_m ) != 0 ) {
  }

  return;
}

/***
 *
 * diskflash_erasetrack
 *
 *	Erases a 4K track of the flash. A track contains 4 1K blocks.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 * parameters:
 *
 *	- track: Supplies the number of the track that is to be erased.
 *
 * return value:
 *
 *	none.
 *
 ***/

void diskflash_erasetrack( unsigned int track )
{
  unsigned int block;

  /* Convert the track number into an address.
   */

  track = track << 12;

  /* erase the four blocks in the track.
   */

  for( block = 0; block < 4; block++ ) {
    diskflash_eraseblock( track + ( block << 10 ) );
  }

  return;

}

/***
 *
 * diskflash_burnsector
 *
 *	This function burns a single sector of the track buffer into
 *	flash. Conveniently, the flash controller provides a block
 *	write operation that can burn 32 longwords (i.e., 128 bytes)
 *	in the time of a single longword.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 * parameters:
 *
 *	- sector: The number of the sector that is to be written. The
 *	  track number is obtained from the track buffer.
 *
 * return value:
 *
 *	none.
 *
 ***/

void diskflash_burnsector( unsigned int sector )
{
  volatile unsigned int *to;
  volatile unsigned int *from;
  unsigned int i;

  /* compute the starting address of the sector in the flash.
   */
 
  cpureg_fma_g = ( diskflash_trackbuffer.track << 12 ) + ( sector << 7 );

  /* move the data.
   */

  to = cpureg_fwb_g;
  from = (unsigned int *)( diskflash_trackbuffer.data + ( sector << 7 ) );
  for( i = 0; i < 32; i++ ) {
    *to++ = *from++;
  }

  /* light off the burn and wait for it to finish.
   */

  cpureg_fmc2_g = cpureg_fmc2_wrbuf_c;
  while( ( cpureg_fmc2_g & cpureg_fmc2_wrbuf_m ) != 0 ) {
  }

  return;
}

/***
 *
 * diskflash_burntrack
 *
 *	This function burns the track buffer to flash. The flash region
 *	is assumed to have been erased.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 * formal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	none.
 *
 ***/

void diskflash_burntrack( void )
{
  unsigned int sector;

  for( sector = 0; sector < 32; sector++ ) {
    diskflash_burnsector( sector );
  }

  return;
}

/***
 *
 * diskflash_buffersector
 *
 *	This function copies a sector from the flash into the track
 *	buffer.
 *
 *	Since we know that both the flash and buffer are longword
 *	aligned, we can transfer 32 bits at a time.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 * parameters:
 *
 *	- track: Supplies the number of the track containing the sector
 *	  to be copied.
 *
 *	- sector: Supplies the number of the sector that is to be
 *	  copied.
 *
 * return value:
 *
 *	none.
 *
 ***/

void diskflash_buffersector( unsigned int track, unsigned int sector )
{
  unsigned int *from;
  unsigned int *to;
  unsigned int i;

  /* calculate starting address of the sector in flash.
   */

  from = (unsigned int *)( ( track << 12 ) + ( sector << 7 ) );

  /* calculate the starting address of the sector in the buffer.
   */

  to = (unsigned int *)( diskflash_trackbuffer.data + ( sector << 7 ) );

  /* copy the sector.
   */

  for( i = 0; i < 32; i++ ) {
    *to++ = *from++;
  }

  return;
}

/***
 *
 * diskflash_buffertrack
 *
 *	The function copies a track from the flash into the track buffer.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 * parameters:
 *
 *	- track: the number of the track to be buffered.
 *
 * return value:
 *
 *	none.
 *
 ***/

void diskflash_buffertrack( unsigned int track )
{
  unsigned int sector;

  for( sector = 0; sector < 32; sector++ ) {
    diskflash_buffersector( track, sector );
  }
  
  diskflash_trackbuffer.track = track;

  return;
}

