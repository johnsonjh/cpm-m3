/************************************************************************
 *
 * file: modubios.h
 *
 *	This file contains declarations for the modular BIOS.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version; consoles only.
 *
 *	2010-08-22 rli: bios descriptor.
 *
 *	2011-06-25 rli: API lists.
 *
 *	2013-05-30 rli: bios_getbios prototype moved to bios.h.
 *
 *	2013-06-01 rli: added tag and version constants. move device
 *	  tables to platform.h.
 *
 ************************************************************************/

#ifndef modubios_h_included
#define modubios_h_included

/*************************
 *
 *	CONSTANTS
 *
 *************************/

/***
 *
 * modubios_tag_c
 *
 *	This tag appears in the BIOS descriptor returned by bios_getbios
 *	to identify the BIOS as modubios.
 *
 * revisions:
 *
 *	2013-06-01 rli: original version.
 *
 ***/

#define modubios_tag_c 0x75646f6d  /* "modu" */

/***
 *
 * modubios_version_c
 *
 *	This constant appears in the BIOS descriptor returned by
 *	bios_getbios to identify the version of modubios.
 *
 * revisions:
 *
 *	2013-06-01 rli: original version.
 *
 ***/

#define modubios_version_c 0x20130601

/*************************
 *
 *	TYPES
 *
 *************************/

/***
 *
 * modubios_api_t
 *
 *	This type describes the generic portion of an API descriptor. A
 *	list of these may be hung off a driver to provide services above
 *	and beyond those accessible in the normal manner. Each API
 *	begins with a header with this layout; clients can locate the
 *	drivers and search their API lists for a desired API.
 *
 *	Beyond this header, the layout of the API descriptor depends
 *	upon the API.
 * 
 * revisions:
 *
 *	2011-06-25 rli: original version.
 *
 * fields:
 *
 *	- tag: Holds a magic number that identifies the API.
 *
 *	- version: Holds a magic number describing the version of the
 *	  API. I tend to use BCD dates for these.
 *
 *	- next: Holds a link to the next API in the list, or NULL if
 *	  this is the last.
 *
 ***/

typedef struct modubios_api_s {
  unsigned int tag;
  unsigned int version;
  struct modubios_api_s *next;
} modubios_api_t;

/***
 *
 * modubios_cons_t
 *
 *	This type describes a console, list, reader, or punch device.
 *	The BIOS contains an array of pointers to these for each class
 *	of device. When an I/O operation is performed, the IOBYTE is
 *	used to select the device to which the operation is directed.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version.
 *
 *	2010-08-08 rli: add a close routine.
 *
 *	2010-08-10 rli: tag and version.
 *
 *	2011-06-25 rli: API list.
 *
 * fields:
 *
 *	- tag: A magic number that can be used to identify this driver.
 *
 *	- version: A magic number used to identify the specific version
 *	  of the driver.
 *
 *	- cboot: This function is called during cold boot. The .bss 
 *	  section has been cleared, but no other initialization has
 *	  been done.
 *
 *	- wboot: This function is called during warm boot.
 *
 *	- close: This function is called when the IOBYTE is changed
 *	  away from this device.
 *
 *	- conin: This function is called to receive a character from
 *	  the device. If no character is available, the function is
 *	  expected to wait.
 *
 *	- conout: This function is called to send a character to the
 *	  device. If the device is not ready to send a character, it
 *	  is expected to wait.
 *
 *	- conist: This function polls a console device to determine
 *	  whether a character has been received.
 *
 *	- conost: This function polls a list device to determine
 *	  whether it is ready to accept a character.
 *
 *	- apilist: A pointer to the first API descriptor in the list of
 *	  additional APIs supported by this driver. If NULL, there are
 *	  no such APIs.
 *
 ***/

typedef struct modubios_cons_s {
  unsigned int tag;
  unsigned int version;
  void (*cboot)( void );
  void (*wboot)( void );
  void (*close)( void );
  char (*conin)( void );
  void (*conout)( char victim );
  unsigned short int (*conist)( void );
  unsigned short int (*conost)( void );
  modubios_api_t *apilist;
} modubios_cons_t;

/***
 *
 * modubios_diskxfer_t
 *
 *	This type describes a disk transfer. It contains the addressing
 *	information needed to figure out what needs to be transferred
 *	and where the buffer resides in memory. A pointer to one of
 *	these is passed to the disk driver's read and write services.
 *
 * revisions:
 *
 *	2010-08-09 rli: original version.
 *
 * fields:
 *
 *	- drive: The number of the currently selected drive.
 *
 *	- track: The number of the track involved in the transfer.
 *
 *	- sector: The number of the sector involved in the transfer.
 *
 *	- buffer: A pointer to the buffer involved in the transfer.
 *
 ***/

typedef struct modubios_diskxfer_s {
  unsigned int drive;
  unsigned int track;
  unsigned int sector;
  void *buffer;
} modubios_diskxfer_t;

/***
 *
 * modubios_disk_t
 *
 *	This structure describes a disk device. The modular BIOS
 *	contains an array of these, one for each possible drive. When
 *	a disk operation is performed, the appropriate driver is found
 *	in the array and passed a description of the operation.
 *
 * revisions:
 *
 *	2010-08-09 rli: original version.
 *
 *	2010-08-10 rli: tag and version.
 *
 *	2010-08-22 rli: pass drive number to wboot, cboot, and flush.
 *
 *	2011-06-25 rli: API list.
 *
 * fields:
 *
 *	- tag: A magic number that can be used to identify this driver.
 *
 *	- version: A magic number used to identify the specific version
 *	  of the driver.
 *
 *	- cboot: This function is called at system initialization time,
 *	  before the BDOS has been initialized.
 *
 *	- wboot: This function is called at warm boot time. A warm boot
 *	  is also performed at system initialization time, after the
 *	  BDOS has been initialized.
 *
 *	- select: This function selects the drive, returning a pointer
 *	  to its drive parameter header.
 *
 *	- read: This function reads a sector from the drive.
 *
 *	- write: This function writes a sector to the drive.
 *
 *	- flush: Instructs the driver to flush its buffers.
 *
 *	- apilist: The header of a linked list of API descriptors for
 *	  each API supported by the driver. If NULL, there are no such
 *	  APIs.
 *
 ***/

typedef struct modubios_disk_s {
  unsigned int tag;
  unsigned int version;
  void (*cboot)( unsigned int drive );
  void (*wboot)( unsigned int drive );
  bios_dph_t *(*select)( unsigned char drive, unsigned char logged );
  unsigned short int (*read)( modubios_diskxfer_t *xfer );
  unsigned short int (*write)( modubios_diskxfer_t *xfer, 
    unsigned short int type );
  unsigned short int (*flush)( unsigned int drive );
  modubios_api_t *apilist;
} modubios_disk_t;

/***
 *
 * modubios_bios_t
 *
 *	This type describes the BIOS. A pointer to a structure can be
 *	fetched by a user program using a BIOS served. The program can
 *	then find all the drivers.
 *
 * revisions:
 *
 *	2010-08-22 rli: original version.
 *
 *	2013-06-01 rli: interchange version and arch fields for
 *	  compatibility with the generic bios_descriptor_t.
 *
 * fields:
 *
 *	- tag: A magic number that identifies the BIOS.
 *
 *	- version: Space for a version tag, should the BIOS wish to
 *	  supply one.
 *
 *	- arch: A magic number that identifies the architecture for
 *	  which the BIOS is written.
 *
 *	- consoles: A pointer to the table of pointers to console
 *	  drivers.
 *
 *	- disks: A pointer to the table of pointers to disk drivers.
 *
 ***/

typedef struct modubios_bios_s {
  unsigned int tag;
  unsigned int version;
  unsigned int arch;
  modubios_cons_t **consoles;
  modubios_disk_t **disks;
} modubios_bios_t;

#endif /* ndef modubios_h_included */
