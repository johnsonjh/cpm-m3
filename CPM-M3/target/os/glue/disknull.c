/*********************************************************************
 *
 * file: disknull.c
 *
 *	This file contains a null disk driver; it is used for requests
 *	directed at a non-existent disk.
 *
 * revisions:
 *
 *	2010-08-10 rli: original version.
 *
 *	2010-08-22 rli: boot and flush routines get drive number.
 *
 *	2011-06-25 rli: ensure we've NULLed out the API list header.
 *
 *	2013-06-08 rli: include config.h so the host tools can #define
 *	  away gcc packing.
 *
 ********************************************************************/

/******************
 *
 *	INCLUDES
 *
 ******************/

#include "config.h"
#include "bios.h"
#include "modubios.h"

/******************
 *
 *	PROTOTYPES
 *
 ******************/

void disknull_boot( unsigned int drive );
bios_dph_t *disknull_select( unsigned char drive, unsigned char logged);
unsigned short int disknull_read( modubios_diskxfer_t *xfer );
unsigned short int disknull_write( modubios_diskxfer_t *xfer,
  unsigned short int type );
unsigned short int disknull_flush( unsigned int drive );

/*****************
 *
 *	GLOBALS
 *
 *****************/

/***
 *
 * disknull
 *
 *	This structure describes the driver to the modular BIOS.
 *
 * revisions:
 *
 *	2010-08-10 rli: original version.
 *
 *	2010-08-10 rli: tag and version.
 *
 *	2011-06-25 rli: API list.
 *
 ***/

modubios_disk_t disknull = {
  0x6c6c756e,		/* "null" */
  0x20100810,
  disknull_boot,	/* cboot */
  disknull_boot,	/* wboot */
  disknull_select,
  disknull_read,
  disknull_write,
  disknull_flush,
  0
};

/*****************
 *
 *	CODE
 *
 *****************/

/***
 *
 * disknull_boot
 *
 *	This function is called for a cold or warm boot. We have no
 *	work to do in this case.
 *
 * revisions:
 *
 *	2010-08-10 rli: original version.
 *
 *	2010-08-22 rli: drive number is passed in.
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

void disknull_boot( unsigned int drive )
{
}

/***
 *
 * disknull_select
 *
 *	This function is called to select the null drive. Since the
 *	drive doesn't exist, we return a NULL pointer for the disk
 *	parameter header address.
 *
 * revisions:
 *
 *	2010-08-10 rli: original version.
 *
 * parmaeters:
 *
 *	- drive: Indicates which drive is being selected.
 *
 *	- logged: Indicates whether the drive has already been logged
 *	  in.
 *
 * return value:
 *
 *	- 0 is always returned to indicate the drive does not exist.
 *
 ***/

bios_dph_t *disknull_select( unsigned char drive, unsigned char logged )
{
  return (bios_dph_t *)0;
}

/***
 *
 * disknull_read
 *
 *	This function is called to read a sector. Since the disk doesn't
 *	exist, we always report an error.
 *
 * revisions:
 *
 *	2010-08-10 rli: original version.
 *
 * parameters:
 *
 *	- xfer: Describes the transfer to be performed.
 *
 * return value:
 *
 *	- 1 is always returned, to indicate that an error occurred.
 *
 ***/

unsigned short int disknull_read( modubios_diskxfer_t *xfer )
{
  return 1;
}

/***
 *
 * disknull_write
 *
 *	This function is called to write a sector. Since the disk
 *	doesn't exist, we always report an error.
 * 
 * revisions:
 *
 *	2010-08-10 rli: original version.
 *
 * parameters:
 *
 *	- xfer: describes the transfer to be performed.
 *
 *	- type: Indicates which type of write is being performed.
 *	  Possibilites are a write to a previously unallocated cluster,
 *	  a directory write, or a write to a previously allocated
 *	  cluster.
 *
 * return value:
 *
 *	- 1 is always returned, to indicate that an error occurred.
 *
 ***/

unsigned short int disknull_write( modubios_diskxfer_t *xfer,
  unsigned short int type )
{
  return 1;
}

/***
 *
 * disknull_flush
 *
 *	This function is called to flush the write buffer. Since the
 *	disk doesn't exist, the buffer hasn't gotten full; we always
 *	report success.
 *
 * revisions:
 *
 *	2010-08-10 rli: original version.
 *
 *	2010-08-20 rli: oops; need to report success, not failure. a
 *	  flush is performed by walking the driver array, flushing
 *	  all drives.
 *
 *	2010-08-22 rli: drive number is passed in.
 *
 * parameters:
 *
 *	- drive: Indicates which drive is being flushed.
 *
 * return value:
 *
 *	- 0 is always returned to indicate the flush succeeded.
 *
 ***/

unsigned short int disknull_flush( unsigned int drive )
{
  return 0;
}

