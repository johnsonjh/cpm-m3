/**********************************************************************
 *
 * file: exchange.c
 *
 *	This file contains stuff stolen from exchange, an earlier
 *	disk image manipulation program.
 *
 * revisions:
 *
 *	2013-12-21 rli: add PUTSYS, which places a file at the beginning
 *	  of the disk image.
 *
 **********************************************************************/

/****************
 *
 *	INCLUDES
 *
 ****************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "cpm.h"
#include "bios.h"
#include "modubios.h"
#include "diverge.h"
#include "flushapi.h"
#include "exchange.h"

/****************
 *
 *	PROTOTYPES
 *
 ****************/

unsigned int cpm_bdos( unsigned short int func, unsigned int parm );

/****************
 *
 *	CODE
 *
 ****************/

/***
 *
 * exchange_findapi
 *
 *	This function searches for a specific API on a specific drive.
 *
 * revisions:
 *
 *	2013-04-26 rli: original version.
 *
 *	2013-06-19 rli: switch to symbolic constants for function numbers.
 *
 * parameters:
 *
 *	- drive: Supplies the number of the drive we're interested in.
 *	  assumed to be in the range [ 0, 15 ].
 *
 *	- tag: Supplies the tag of the API we're looking for.
 *
 * results:
 *
 *	- NULL: The drive does not support the specified API.
 *
 *	- else: A pointer to the specified API descriptor for the
 *	  specified drive.
 *
 * globals:
 *
 *	- modubios_tag_c: Identifies a BIOS as being a modubios.
 *
 ***/

modubios_api_t *exchange_findapi( unsigned int drive, unsigned int tag )
{
  modubios_bios_t *modubios;
  modubios_api_t *moduapi;
  modubios_disk_t *disk;
  cpm_bpb_t bpb;

  /* ask the bios for a pointer to the bios descriptor. */

  bpb.func = bios_getbios_c;
  modubios = (modubios_bios_t *)( bdos( bdos_callbios_c, (unsigned int)&bpb ) );

  /* if the BIOS doesn't understand function 23, it can't be a modubios. */

  if( modubios == (void *)0 ) {
    return (void *)0;
  }

  /* or it could be something other than a modubios. */

  if( modubios->tag != modubios_tag_c ) {
    return (void *)0;
  }

  /* copy the pointer to the disk descriptor somewhere handy (less typing) */

  disk = modubios->disks[ drive ];

  /* start with the first api, which is hanging off the disk descriptor. */

  moduapi = disk->apilist;
  while( moduapi != (void *)0 ) {

    /* is this the API we're looking for? */

    if( moduapi->tag == tag ) return moduapi;

    /* nope. advance to the next one. */

    moduapi = moduapi->next;

  }

  /* we walked the whole list without finding the api; this drive doesn't
   * support it.
   */

  return (void *)0;

}

/***
 *
 * exchange_flushdrive
 *
 *	Flushes a single drive, causing its backing file to be updated.
 *
 * revisions:
 *
 *	2013-04-26 rli: original version.
 *
 * parameters:
 *
 *	- drive: Indicates which drive is to be flushed. The range limit
 *	  of [ 0, 15 ] is rudely enforced.
 *
 * globals:
 *
 *	- flushapi_tag_c: Identifies the flush API.
 *
 ***/

void exchange_flushdrive( unsigned int drive )
{
  flushapi_t *flushapi;

  /* rudely enforce the drive number range. */

  drive = drive & 0xf;

  /* find the flush API for this drive. */

  flushapi = (flushapi_t *)( exchange_findapi( drive, flushapi_tag_c ) );

  /* if the drive doesn't support the flush api, we can't flush it. */

  if( flushapi == (void *)0 ) {
    return;
  }

  /* flush the drive. */

  flushapi->flush( drive );
  printf( "flushed drive %c:\n", drive + 65 );

  return;
}

/***
 *
 * exchange_flushdrives
 *
 *	This function flushes all of the drives.
 *
 * revisions:
 * 
 *	2013-04-26 rli: original version.
 *
 * paramters:
 *
 *	none.
 *
 ***/

void exchange_flushdrives( void )
{
  unsigned int i;

  for( i = 0; i < 16; i++ ) {
    exchange_flushdrive( i );
  }

  return;
}

/***
 *
 * exchange_exit
 *
 *	This function exits. Eventually it will flush the disk images
 *	before doing so.
 *
 * revisions:
 *
 *	2013-04-26 rli: liberated from exchange and worked over.
 *
 *	2013-04-26 rli: flush the drives before exiting.
 *
 * parameters:
 *
 *	none.
 *
 ***/

void exchange_exit( void )
{
  exchange_flushdrives();
  exit( EXIT_SUCCESS );
}

/***
 *
 * exchange_fcb2path
 *
 *	Extracts the filename from an FCB, converting it to a UNIX file
 *	name.
 *
 * revisions:
 *
 *	2013-04-26 rli: liberated from exchanged and worked over.
 *
 * parameters:
 *
 *	- fcb: Supplies a pointer to the FCB from which the path is
 *	  to be extracted.
 *
 *	- path: Supplies a pointer to the buffer that is to receive
 *	  the filename. This is assumed to be large enough to hold the
 *	  name (8.3 + terminal null = 13 characters).
 *
 * return values:
 *
 *	- 0: The filename in the FCB is blank; the extracted path is ".".
 *
 *	- 1: A filename has been extracted from the FCB.
 *
 ***/

int exchange_fcb2path( cpm_fcb_t *fcb, char *path )
{
  int i;
  int pathindex = 0;

  for( i = 0; i < 8; i++ ) {
    if( ( fcb->f[ i ] & 0xff ) < '!' ) break;
    path[ pathindex++ ] = fcb->f[ i ];
  }
  path[ pathindex++ ] = '.';
  for( i = 0; i < 3; i++ ) {
    if( ( fcb->t[ i ] & 0xff ) < '!' ) break;
    path[ pathindex++ ] = fcb->t[ i ];
  }
  path[ pathindex ] = 0;
  if( pathindex == 1 ) return 0;
  return 1;
}

/***
 *
 * exchange_import
 *
 *	Copies a file into CP/M-land. This is intended to be called
 *	by the CCP as part of its command handling.
 *
 * revisions:
 *
 *	2013-04-26 rli: liberated from exchange and worked over.
 *
 *	2013-06-19 rli: switch to symbolic constants for function numbers.
 *
 * parameters:
 *
 *	none.
 *
 ***/

void exchange_import( void )
{
  extern unsigned short int cpm_fill_fcb(
    unsigned short int which_parm,
    cpm_fcb_t *fcb );
  int i;
  cpm_fcb_t fcb;
  int ambiguous;
  char path[ 13 ];
  FILE *exportfile;
  char buffer[ 128 ];

  /* fill our FCB from the next parameter in the command line. */

  ambiguous = cpm_fill_fcb( 1, &fcb );
  if( ambiguous ) {
    printf( "Wildcards not supported." );
    return;
  }

  /* construct a host filename from the FCB */

  if( !exchange_fcb2path( &fcb, path ) ) {
    printf( "Cannot import ." );
    return;
  }

  /* attempt to delete the file if it exists; it's ok if this fails,
   * because make file will complain if it's still there.
   */

  cpm_bdos( bdos_deletefile_c, (unsigned long int)&fcb );

  /* attempt to create the file. */

  if( cpm_bdos( bdos_createfile_c, (unsigned long int)&fcb ) > 3 ) { 
    printf( "Cannot make " );
    for( i = 0; i < 8; i++ )
      printf( "%c", fcb.f[ i ] & 0x7f );
    printf( "." );
    for( i = 0; i < 3; i++ )
      printf( "%c", fcb.t[ i ] & 0x7f );
    return;
  }

  /* open the host file that we're going to import. */

  exportfile = fopen( path, "rb" );
  if( exportfile == NULL ) {
    perror( path );
    return;
  }

  /* tell the user what we're doing. */

  printf( "import %s", path );
  printf( " -> " );
  for( i = 0; i < 8; i++ )
    printf( "%c", fcb.f[ i ] & 0x7f );
  printf( "." );
  for( i = 0; i < 3; i++ )
    printf( "%c", fcb.t[ i ] & 0x7f );

  /* set the CP/M DMA address to our buffer. */

  cpm_bdos( bdos_setdmaaddress_c, (unsigned long int)&buffer );

  /* copy blocks from the host to CP/M until we're either done or 
   * encounter a problem.
   *
   * since CP/M doesn't track filesizes to the byte, fill the buffer
   * with ^Zs first to ensure that the end of a text file is properly
   * padded.
   */

  while( 1 ) {
    memset( buffer, 0x1a, 128 );
    if( fread( buffer, 1, 128, exportfile ) == 0 ) {
      break;
    }
    i = cpm_bdos( bdos_writesequential_c, (unsigned long int)&fcb );
    if( i == 1 ) {
      printf( " Directory full." );
      break;
    }
    if( i == 2 ) {
      printf( " Disk full." );
      break;
    }
    if( i != 0 ) {
      printf( " Unknown error %d", i );
      break;
    }
  }

  /* close the CP/M and host files. */

  cpm_bdos( bdos_closefile_c, (unsigned long int)&fcb );
  fclose( exportfile );

}

/***
 *
 * exchange_export
 *
 *	Copies a file from CP/M-land to the host. It is intended to be
 *	called by the CCP as part of its command processing.
 *
 * revisions:
 *
 *	2013-04-26 rli: liberated from exchange and worked over.
 *
 *	2013-06-19 rli: switch to symbolic constants for bdos function numbers.
 *
 * parameters:
 *
 *	none.
 *
 ***/

void exchange_export( void )
{
  extern unsigned short int cpm_fill_fcb(
    unsigned short int which_parm,
    cpm_fcb_t *fcb );
  int i;
  cpm_fcb_t fcb;
  int ambiguous;
  char path[ 13 ];
  FILE *exportfile;
  char buffer[ 128 ];

  /* fill an FCB with the next filename from the command line. */

  ambiguous = cpm_fill_fcb( 1, &fcb );
  if( ambiguous ) {
    printf( "Wildcards not supported." );
    return;
  }

  /* extract the filename from the FCB to a Unix filename */

  if( !exchange_fcb2path( &fcb, path ) ) {
    printf( "Cannot export ." );
    return;
  }

  /* Attempt to open the CP/M file. */

  if( cpm_bdos( bdos_openfile_c, (unsigned long int)&fcb ) > 3 ) {
    printf( "No file." );
    return;
  }

  /* Attempt to create the Unix file. */

  exportfile = fopen( path, "wb" );
  if( exportfile == NULL ) {
    perror( path );
    return;
  }

  /* let the user know what we're up to. */

  printf( "export " );
  for( i = 0; i < 8; i++ )
    printf( "%c", fcb.f[ i ] & 0x7f );
  printf( "." );
  for( i = 0; i < 3; i++ )
    printf( "%c", fcb.t[ i ] & 0x7f );
  printf( " -> " );
  printf( "%s", path );

  /* point the CP/M DMA address at our buffer. */

  cpm_bdos( bdos_setdmaaddress_c, (unsigned long int)&buffer );

  /* Copy the file until we've either run out of data or couldn't write
   * something.
   */

  while( 1 ) {
    if( cpm_bdos( bdos_readsequential_c, (unsigned long int)&fcb ) != 0 ) {
      break;
    }
    if( fwrite( buffer, 1, 128, exportfile ) != 128 ) {
      perror( "short write" );
      break;
    }
  }

  /* close both the CP/M and Unix files. */

  cpm_bdos( bdos_closefile_c, (unsigned long int)&fcb );
  fclose( exportfile );

}

/***
 *
 * exchange_putsys
 *
 *	Copies a file into the system area (i.e., the first part) of the
 *	disk image.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 * parameters:
 *
 *	none.
 *
 ***/

void exchange_putsys( void )
{
  extern unsigned short int cpm_fill_fcb(
    unsigned short int which_parm,
    cpm_fcb_t *fcb );
  int i;
  cpm_fcb_t fcb;
  int ambiguous;
  char path[ 13 ];
  FILE *sysfile;
  flushapi_t *flushapi;
  int drive;
  unsigned char *diskbuffer;

  /* fill an FCB with the next filename from the command line. */

  ambiguous = cpm_fill_fcb( 1, &fcb );
  if( ambiguous ) {
    printf( "Wildcards not supported." );
    return;
  }

  /* extract the filename from the FCB to a Unix filename */

  if( !exchange_fcb2path( &fcb, path ) ) {
    printf( "Cannot export ." );
    return;
  }

  /* figure out with which drive we're dealing.
   */

  if( fcb.dr == 0 ) {

    /* default drive; we need to ask the BDOS.
     */

    drive = cpm_bdos( bdos_getcurrentdisk_c, 0 );

  } else {

    /* fcb specifies drive number, but it's one-based.
     */

    drive = fcb.dr - 1;
  }

  /* find the flush API for this drive. */

  flushapi = (flushapi_t *)( exchange_findapi( drive, flushapi_tag_c ) );

  /* if the drive doesn't support the flush api, we can't find its
   * disk buffer.
   */

  if( flushapi == (void *)0 ) {
    printf( "Drive %c: does not have a flushapi.", drive + 'A' );
    return;
  }

  /* Find the drive's disk buffer. If it doesn't have one, we can't
   * put a system in it.
   */

  diskbuffer = flushapi->bufaddr( drive );
  if( diskbuffer == (void *)0 ) {
    printf( "Drive %c: does not have an image buffer.", drive + 'A' );
    return;
  }

  /* Open the system image.
   */

  sysfile = fopen( path, "rb" );
  if( sysfile == NULL ) {
    perror( path );
    return;
  }

  /* tell the user what we're doing. */

  printf( "putsys %s -> %c:", path, drive + 'A' );

  /* Copy the file to the disk image buffer.
   */

  while( 1 ) {
    if( fread( diskbuffer, 1, 1, sysfile ) != 1 ) break;
    diskbuffer++;
  }

  /* Close the file and mark the image as dirty.
   */

  fclose( sysfile );
  flushapi->setdirty( drive );

  return;
}

