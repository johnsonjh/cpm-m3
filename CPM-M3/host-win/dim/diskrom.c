/*********************************************************************
 *
 * file: diskrom.c
 *
 *	This is a RAMdisk device driver that manipulates an image of
 *	a disk intended to be burned into the processor's flash.
 *
 *	The flash is 512K. We're reserving the first 64K for the system
 *	image. The remainder of the flash will be used as disk.
 *
 * revisions:
 *
 *	2010-08-11 rli: snipped from HawkBios and starting fiddling
 *	  about.
 *
 *	2010-08-22 rli: boot and flush routines get drive number.
 *
 *	2011-06-25 rli: null out the API list pointer.
 *
 *	2013-04-26 rli: liberated from arm-hawk and started fussing
 *	  about.
 *
 *	2013-05-01 rli: extend disk to cover the whole 512K, reserving
 *	  the first 64K for the system.
 *
 *	2013-06-16 rli: tweaked includes.
 *
 *	2013-12-21 rli: mechanism to report address of the disk buffer.
 *
 *	2013-12-28 rli: 4K tracks, reserve only 32K for the system.
 *
 *********************************************************************/

/***********************
 *
 *	INCLUDES
 *
 ***********************/

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "bios.h"
#include "modubios.h"
#include "flushapi.h"

/***********************
 *
 *	PROTOTYPES
 *
 ***********************/

void diskrom_cboot( unsigned int drive );
void diskrom_wboot( unsigned int drive );
bios_dph_t *diskrom_select( unsigned char drive, unsigned char logged ); 
unsigned short int diskrom_read( modubios_diskxfer_t *xfer );
unsigned short int diskrom_write( modubios_diskxfer_t *xfer,
  unsigned short int type );
unsigned short int diskrom_flush( unsigned int drive );

void diskrom_flushimage( unsigned int drive );
int diskrom_imageisdirty( unsigned int drive );
void *diskrom_bufaddr( unsigned int drive );
void diskrom_setdirty( unsigned int drive );

/***********************
 *
 *	CONSTANTS
 *
 ***********************/

/***
 *
 * diskrom_imagename
 *
 *	Supplies the name of the image file that is used to initialize
 *	the disk at boot time and to which the disk is flushed.
 *
 * revisions:
 *
 *	2013-04-26 rli: original version.
 *
 ***/

#define diskrom_imagename "romdisk.img"

/***********************
 *
 * 	GLOBALS
 *
 ***********************/

/***
 *
 * diskrom_flushapi
 *
 *	This API descriptor provides a mechanism for the CCP to inform
 *	us that it's time to write the image to the backing file.
 *
 * revisions:
 *
 *	2013-04-26 rli: original version.
 *
 *	2013-12-21 rli: added bufaddr and setdirty.
 *
 ***/

flushapi_t diskrom_flushapi = {
  { flushapi_tag_c,
    flushapi_version_c,
    (void *)0
  },
  diskrom_flushimage,
  diskrom_imageisdirty,
  diskrom_bufaddr,
  diskrom_setdirty
};

/***
 *
 * diskrom
 *
 *	This structure describes the driver to the modular BIOS.
 *
 * revisions:
 *
 *	2010-08-11 rli: original version. don't need to distinguish
 *	  between warm and cold boot.
 *
 *	2011-06-25 rli: added API list pointer.
 *
 *	2013-04-26 rli: retag for use as ALID 4 flash driver. separate
 *	  warm and cold boot.
 *
 ***/

modubios_disk_t diskrom = {
  0x006d6f72,		/* "rom" */
  0x20130426,
  diskrom_cboot,
  diskrom_wboot,
  diskrom_select,
  diskrom_read,
  diskrom_write,
  diskrom_flush,
  &diskrom_flushapi.generic
};

/***
 *
 * diskrom_dpb
 *
 *	This disk parameter block describes the disk to the system.
 *	We have 448KB to play with, so we have to use 2KB clusters.
 *
 * revisions:
 *
 *	2013-04-26 rli: reworked for ALID IV flash image. 2k per track,
 *	  because I can. Since we'll be loading the image from a single
 *	  fixed file at startup, we can treat it as a hard disky (no
 *	  media swaps, don't need a check vector).
 *
 *	2013-05-01 rli: expand drive to cover the whole 512K, with
 *	  64K reserved for the system.
 *
 *	2013-12-28 rli: switch to 4K per track, reserve only 32K for
 *	  the system.f
 *
 ***/

bios_dpb_t diskrom_dpb = {
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
 * diskrom_alv
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

unsigned char diskrom_alv[ ( 223 / 8 ) + 1 ];

/***
 *
 * diskrom_dph
 *
 *	This is the disk parameter header for the 8" SSSD RAMdisk. When
 *	the BIOS selects the drive, it receives a pointer to this
 *	structure, which it uses to locate the others.
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

bios_dph_t diskrom_dph = {
  0,			/* sector translation table */
  0, 0, 0, 		/* scratch words */
  bios_dirbuf,		/* directory buffer */
  &diskrom_dpb,		/* disk parameter block */
  0,			/* checksum vector */
  diskrom_alv		/* allocation vector */
};

/***
 *
 * diskrom_image
 *
 *	This is the disk image.
 *
 * revisions:
 *
 *	2010-08-11 rli: original, for an image built by ../tools/mkimage.
 *
 *	2010-08-22 rli: use object file created by objcopy.
 *
 *	2013-04-26 rli: reserve a big buffer here.
 *
 *	2013-05-01 rli: expand to cover the whole 512K; the first 64K
 *	  will be reserved for the system.
 *
 ***/

char diskrom_image[ 8 * 64 * 1024 ];

/***
 *
 * diskrom_dirty
 *
 *	This flag keeps track of whether the disk has been written since
 *	it was last flushed to the image file.
 *
 ***/

int diskrom_dirty = 0;

/*******************
 *
 *	CODE
 *
 *******************/

/***
 *
 * diskrom_cboot
 *
 *	This function is called for a cold boot. It loads the image from
 *	its disk file.
 *
 * revisions:
 *
 *	2013-04-23 rli: original version.
 *
 * parameters:
 *
 *	- drive: Indicates which drive is being initialized.
 *
 ***/

void diskrom_cboot( unsigned int drive )
{
  FILE *imagefile;

  imagefile = fopen( diskrom_imagename, "rb" );
  if( imagefile == (void *)0 ) {
    printf( "diskrom: could not open %s; image not loaded.\n", 
      diskrom_imagename );
    memset( diskrom_image, 0xe5, sizeof( diskrom_image ) );
    return;
  }

  if( fread( diskrom_image, sizeof( diskrom_image ), 1, imagefile ) != 1 ) {
    printf( "diskrom: could not load image from %s.\n",
      diskrom_imagename );
    memset( diskrom_image, 0xe5, sizeof( diskrom_image ) );
    return;
  }

  printf( "diskrom: drive %c: loaded from %s.\n",
    drive + 65, diskrom_imagename );
  diskrom_dirty = 0;
  return;
}

/***
 *
 * diskrom_wboot
 *
 *	This function is called for a warm boot. 
 *
 * revisions:
 *
 *	2010-08-11 rli: original version.
 *
 *	2010-08-22 rli: drive number is now passed in.
 *
 *	2013-04-26 rli: print a message if the drive is dirty.
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

void diskrom_wboot( unsigned int drive )
{
  if( diskrom_dirty ) {
    printf( "\n\n(diskrom: drive %c: is dirty.)", 65 + drive );
  }
  return;
}

/***
 *
 * diskrom_select
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

bios_dph_t *diskrom_select( unsigned char drive, unsigned char logged )
{
  return &diskrom_dph;
}

/***
 *
 * diskrom_flush
 *
 *	This function is called when it's time to flush the buffers.
 *	Since we don't have a buffer, we don't need to do anything.
 *
 * revisions:
 *
 *	2010-08-11 rli: original version.
 *
 *	2010-08-22 rli: drive number is passed in.
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

unsigned short int diskrom_flush( unsigned int drive )
{
  return 0;
}

/***
 *
 * diskrom_movesector
 *
 *	This function moves a sector of data from one location to
 *	another.
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

void diskrom_movesector( unsigned char *from, unsigned char *to )
{
  int count;

  for( count = 0; count < 128; count++ )
    *to++ = *from++;
}

/***
 *
 * diskrom_read
 *
 *	This function is called to read a sector from the disk.
 *
 * revisions:
 *
 *	2010-08-11 rli: original version.
 *
 *	2013-04-26 rli: update for 16 sectors per track.
 *
 *	2013-12-28 rli: use dpb for sectors per track instead of
 *	  a hard-coded constant.
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

unsigned short int diskrom_read( modubios_diskxfer_t *xfer )
{
  unsigned int offset;

  offset = ( ( xfer->track * diskrom_dpb.spt ) + xfer->sector ) * 128;

  diskrom_movesector( 
    &diskrom_image[ offset ], 
    (unsigned char *)xfer->buffer );

  return 0;

}

/***
 *
 * diskrom_write
 *
 *	This function is called to write a sector to the disk.
 *
 * revisions:
 *
 *	2010-08-11 rli: original version.
 *
 *	2013-04-26 rli: mark the drive as dirty. update for 16 sectors
 *	  per track.
 *
 *	2013-12-28 rli: get sectors per track from the dpb instead
 *	  of a constant.
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
 *	- 0 is always returned to indicate that the operation succeeded.
 *
 ***/

unsigned short int diskrom_write( modubios_diskxfer_t *xfer,
  unsigned short int type )
{
  unsigned int offset;

  offset = ( ( xfer->track * diskrom_dpb.spt ) + xfer->sector ) * 128;

  diskrom_movesector( 
    (unsigned char *)xfer->buffer,
    &diskrom_image[ offset ] ); 

  /* the drive is now dirty. */

  diskrom_dirty = 1;

  return 0;
}

/***
 *
 * diskrom_flushimage
 *
 *	Copies the disk image to the backing file.
 *
 * revisions:
 *
 *	2013-04-26 rli: original version.
 *
 * parameters:
 *
 *	- drive: Not used.
 *
 ***/

void diskrom_flushimage( unsigned int drive )
{
  FILE *imagefile;

  imagefile = fopen( diskrom_imagename, "wb" );
  if( imagefile == (void *)0 ) {
    printf( "diskrom: could not create %s; image not flushed.\n", 
      diskrom_imagename );
    return;
  }

  if( fwrite( diskrom_image, sizeof( diskrom_image ), 1, imagefile ) != 1 ) {
    printf( "diskrom: could not save image to %s.\n",
      diskrom_imagename );
    return;
  }

  printf( "diskrom: drive %c: saved to %s.\n",
    drive + 65, diskrom_imagename );

  diskrom_dirty = 0;

  return;
}

/***
 *
 * diskrom_imageisdirty
 *
 *	This function allows the CCP to find out whether the image has
 * 	been written since the last flush.
 *
 * revisions:
 *
 *	2013-04-26 rli: original version.
 *
 * parameters:
 *
 *	- drive: not used.
 *
 ***/

int diskrom_imageisdirty( unsigned int drive )
{
  return diskrom_dirty;
}

/***
 *
 * diskrom_bufaddr
 *
 *	This function allows the CCP to find our image buffer for special
 *	operations such as PUTSYS.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 * parameters:
 *
 *	- drive: not used.
 *
 ***/

void *diskrom_bufaddr( unsigned int drive )
{
  return diskrom_image;
}

/***
 *
 * diskrom_setdirty
 *
 *	This function allows the CCP to mark the disk buffer as dirty.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 ***/

void diskrom_setdirty( unsigned int drive )
{
  diskrom_dirty = 1;
  return;
}

