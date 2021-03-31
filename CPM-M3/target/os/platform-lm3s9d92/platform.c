/*******************************************************************
 *
 * file: platform.c
 *
 *	This file provides a home for platform-specific data structures.
 *
 * revisions:
 *
 *	2013-12-21 rli: added comments.
 *
 *******************************************************************/

#include "config.h"
#include "cpm.h"
#include "bios.h"
#include "modubios.h"
#include "platform.h"

/*******************
 *
 *	GLOBALS intended for PUBLIC use
 *
 *******************/

/***
 *
 * platform_mrt
 *
 *	The memory region table. Describes the available memory
 *	regions.
 *
 * revisions:
 *
 *	2013-12-21 rli: default initialization representing a 64KB TPA.
 *
 ***/

bios_mrt_t platform_mrt = {
  1,
  { { (void *)0x20000000, 65536 } }
};

/***
 *
 * platform_consoles
 *
 *	Holds pointers to the descriptors for each of the console
 *	device drivers.
 *
 * revisions:
 *
 *	2013-12-21 rli: added consuart0.
 *
 ***/

extern modubios_cons_t consnull;
extern modubios_cons_t consuart0;

modubios_cons_t *platform_consoles[ 16 ] = {
  &consuart0, &consnull, &consnull, &consnull,
  &consnull,  &consnull, &consnull, &consnull,
  &consnull,  &consnull, &consnull, &consnull,
  &consnull,  &consnull, &consnull, &consnull
};

/***
 *
 * platform_disks
 *
 *	Holds pointers to the descriptors for each of the disk device
 *	drivers.
 *
 * revisions:
 *
 *	2013-12-21 rli: diskflash as drive a:
 *
 ***/

extern modubios_disk_t disknull;
extern modubios_disk_t diskflash;

modubios_disk_t *platform_disks[ 16 ] = {
  &diskflash, &disknull, &disknull, &disknull,
  &disknull,  &disknull, &disknull, &disknull,
  &disknull,  &disknull, &disknull, &disknull,
  &disknull,  &disknull, &disknull, &disknull
};


