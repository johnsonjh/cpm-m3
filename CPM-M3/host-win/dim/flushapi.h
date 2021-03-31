/**********************************************************************
 *
 * file: flushapi.h
 *
 *	This file declares an API that allows the system to flush disk image
 *	drivers to their backing files and allows those drivers to
 *	inform the system about whether they have been written to since
 *	the last flush.
 *
 * revisions:
 *
 *	2013-04-26 rli: original version.
 *
 *	2013-12-21 rli: provide a mechanism to report the address of the
 *	  disk buffer and set the dirty flag for special operations like
 *	  PUTSYS.
 *
 **********************************************************************/

#ifndef flushapi_h_included
#define flushapi_h_included

/************************
 *
 *	INCLUDES
 *
 ************************/

#include "modubios.h"

/************************
 *
 *	CONSTANTS
 *
 ************************/

/***
 *
 * flushapi_tag_c
 *
 *	This magic number tags an API descriptor as describing this API.
 *
 * revisions:
 *
 *	2013-04-26 rli: original version.
 *
 ***/

#define flushapi_tag_c 0x68736c66 /* "flsh" */

/***
 *
 * flushapi_version_c
 *
 *	This number identifies the version of this API.
 *
 * revisions:
 *
 *	2013-04-26 rli: original version.
 *
 *	2013-12-21 rli: bumped.
 *
 ***/

#define flushapi_version_c 0x20131221

/************************
 *
 *	TYPES
 *
 ************************/

/***
 *
 * flushapi_t
 *
 *	This structure describes the API and provides pointers to the
 *	necessary functions.
 *
 * revisions:
 *
 *	2013-04-26 rli: original version.
 *
 *	2013-12-21 rli: bufaddr; the pointer returned is the address of
 *	  the disk image.
 *
 ***/

typedef struct flushapi_s {
  modubios_api_t generic;
  void (*flush)( unsigned int drive );
  int (*isdirty)( unsigned int drive );
  void *(*bufaddr)( unsigned int drive );
  void (*setdirty)( unsigned int drive );
} flushapi_t;

#endif /* ndef flushapi_h_included */
