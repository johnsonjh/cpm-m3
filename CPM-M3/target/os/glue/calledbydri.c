/************************************************************************
 *
 * file: calledbydri.c
 *
 *	This bits of glue are referred to by the DRI code. The names and
 *	interfaces of these functions are therefore controlled by the
 *	expectations of that code.
 *
 * revisions:
 *
 *	2013-05-27 rli: split from arch.c
 *
 *	2013-05-30 rli: switch to platform-specific constants to supply
 *	  the extension of the .COM-style executables. Rename
 *	  glue_loadarm to the more generic glue_loadcom.
 *
 *	2013-06-01 rli: need modubios.h for platform.h
 *
 *	2013-06-04 rli: moved configuration constants to config.h; we
 *	 no longer need platform.h here, which means we don't need
 *	 modubios.h.
 *
 *	2013-06-05 rli: allow the default do-nothing trap handler to
 *	  be overridden.
 *
 *	2014-01-01 rli: trim the space reserved for the base page to
 *	  256 bytes to comport with tradition and the CP/M-68K documentation,
 *	  but mostly the CP/M-68K documentation.
 *
 ***********************************************************************/

/******************
 *
 *	INCLUDES
 *
 ******************/

#include "config.h"
#include "cpm.h"
#include "bios.h"
#include "glue.h"

/***********************
 *
 *	PROTOTYPES
 *
 ***********************/

unsigned short int _bdos( unsigned short int func, unsigned short int info, void *infop );

/***********************
 *
 *	CODE
 *
 ***********************/

/***
 *
 * bdos
 *
 *	A user program that calls the BDOS goes through here. Among
 *	other things, this gives us a whack at the call so we can peel
 *	off functions performed here (such as 50 DIRECT BIOS CALL). 
 *	If it's not a function performed by the BIOS, it is passed on to 
 *	the actual BDOS.
 *
 * revisions:
 *
 *	2005-03-05 rli: original attempt.
 *
 *	2010-08-02 rli: genericized structure names.
 *
 *	2010-08-28 rli: stretch return value to a 32-bit int because
 *	  direct BIOS call requires it; that seems to be the only BDOS
 *	  call that needs to return a 32-bit value, though.
 *
 *	2013-05-03 rli: use bdos function numbers from cpm.h.
 *
 *	2013-05-29 rli: renamed functions that now live in glue.c.
 *
 * formal parameters:
 *
 *	- func: The number of the BDOS function to be performed.
 *
 *	- parm: The parameter to be passed to that function, if one is
 *	  is needed. 
 *
 * informal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	whatever is returned by the BDOS call.
 *
 ***/

unsigned int bdos( unsigned short int func, unsigned int parm )
{
  switch( func ) {
    case bdos_callbios_c:     return glue_callbios( (cpm_bpb_t *)parm );
    case bdos_enterprogram_c: return glue_enterprogram(
                                (cpm_basepage_t *)parm );
    default:                  return _bdos( func, (unsigned short int)parm,
                                (void *)parm );
  }
}

/***
 *
 * udiv
 *
 *	This routine is used by the BDOS to do some division,
 *	simultaneously obtaining both the quotient and the remainder.
 *
 * revisions:
 *
 *	2005-03-07 rli: Added these comments.
 *
 * formal parameters:
 *
 *	- dividend: The dividend.
 *
 *	- divisor: The divisor.
 *
 *	- remainder: A pointer to somewhere to store the remainder.
 *
 * informal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	the quotient.
 *
 * side effects:
 *
 *	none.
 *
 ***/

unsigned short int udiv( 
  signed int dividend, 
  unsigned short int divisor,
  unsigned short int *remainder )
{
  *remainder = dividend % divisor;
  return dividend / divisor;
}

/***
 *
 * swap
 *
 *	This routine is used by the BDOS to swap words read from the
 *	disk (e.g., from the allocation map) into host byte order.
 *
 *	The code contained herein figures out what type of swapping
 *	should be performed the first time it is called. This can
 *	be hard-coded for a particular system, if you like.
 *
 * revisions:
 *
 *	2005-03-07 rli: Added these comments
 *
 * formal parameters:
 *
 *	- victim: The word to be swapped.
 *
 * informal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	The swapped word.
 *
 * side effects:
 *
 *	none.
 *
 ***/

unsigned short int swap( unsigned short int victim )
{
  static int swaptype= -1;
  static unsigned short int testpattern=0x0102;
  unsigned short int temp;

  if( swaptype < 0 ) {
    if( *(char *)&testpattern == 0x01 ) {
      swaptype = 1;
    } else {
      swaptype = 0;
    }
  }

  if( swaptype == 0 ) return victim;

  temp = ( (victim & 0xff ) << 8 ) |
    ( ( victim & 0xff00 ) >> 8 );
  return temp;
}

/***
 *
 * Miscellaneous CCP variables.
 *
 *	These variables are declared and initialized outside the CCP.
 *
 *	The submit and morecmds flags are involved in a complicated
 *	dance dealing with submit files and command lines that are split
 *	by an exclamation point.
 *
 *	The autost flag in combination with an autorom flag that is not
 *	declared extern by the ccp seem to control whether the CCP
 *	executes the command line stored in usercmd.
 *
 * revisions:
 *
 *	2013-05-27 rli: added a date to this comment block and a
 *	  slightly less vague description of the variables.
 *
 ***/

unsigned char submit = 0;
unsigned char morecmds = 0;
unsigned char autost = 0;
signed char usercmd[ 130 ];

/***
 *
 * pgmld
 *
 *	BDOS FUNCTION 59: LOAD PROGRAM
 *
 *	This routine implements BDOS function 59, which loads a program
 *	into a specified region of memory.
 *
 *	This initial implementation deals only with .COM-style
 *	executables, files that contain a dump of memory beginning
 *	256 bytes past the start of the TPA.
 *
 * revisions:
 *
 *	2005-03-05 rli: original version.
 *
 *	2010-08-02 rli: genericized structure names.
 *
 *	2010-08-16 rli: strip checks; assume program is properly linked.
 *	  delete initialization of program section addresses; i'm
 *	  thinking of removing them from the base page.
 *
 *	2010-08-19 rli: basepage rework for CP/M-68K and CP/M-8K
 *	  compatibility.
 *
 *	2011-03-06 rli: Forgot to initialize loadedfrom in the base
 *	  page. It's taken from the first byte of the FCB, unless it's
 *	  0; in that case, the current default drive +1 is used.
 *
 *	2013-05-03 rli: use bdos function number constants from cpm.h.
 *
 *	2013-12-22 rli: we need the header again, in order to pick up
 *	  the entry point address (which can't be calculated in an
 *	  architecture-independent manner; Thumb needs bit 0 set, other
 *	  archs don't).
 *
 *	2014-01-01 rli: trim the space reserved for the base page to
 *	  256 bytes to comport with tradition.
 *
 *	2014-01-04 rli: return codes 4 and 5.
 *
 * formal parameters:
 *
 *	- infop: A pointer to a cpm_lpb_t describing the memory region into 
 *	  which the program is loaded.
 *
 *	- dmaaddress: The current default DMA address. This is used
 *	  primarily to restore that default after the program is
 *	  loaded.
 *
 * informal parameters:
 *
 *	- config_archtag_c: The value we expect to find in the tag field
 *	  of an executable header.
 *
 * return value:
 *
 *	- 0: Success. The program has been loaded and may be entered
 *	  after the remainder of the base page has been initialized.
 *
 *	- 1: Insufficient memory to load the program, a bad program
 *	  header (not applicable here), or the load parameter
 *	  block is not acceptable.
 *
 *	- 2: A read error occurred while loading the file into memory.
 *	     (except that the BDOS doesn't report disk errors...)
 *
 *	- 3: Bad relocation bits exist in the program file. 
 *
 *	- 4: Wrong architecture. The executable header does not contain
 *	  the expected tag value.
 *
 *	- 5: Wrong region. The executable header indicates that the
 *	  program wants to be loaded at some other address.
 *
 ***/

unsigned short int pgmld( void *infop, void *dmaaddress )
{
  cpm_lpb_t *lpb = (cpm_lpb_t *)infop;
  unsigned short int ReadStatus;
  unsigned int NextSector;
  cpm_exehdr_t *exehdr;

  /* Initialize the base page pointer.
   */

  lpb->basepage = (void *)lpb->tpabase;

  /* Load the file into the TPA a sector at a time.
   */

  for( NextSector = (unsigned int)lpb->tpabase + 0x100; 
    NextSector < (unsigned int)lpb->tpatop; NextSector += 128 ) {

    /* First, set the DMA address to the place we want to store this
     * sector.
     */

    bdos( bdos_setdmaaddress_c, NextSector );

    /* Read the sector.
     */

    ReadStatus = bdos( bdos_readsequential_c, (unsigned int)lpb->fcb );
    if( ReadStatus != 0 ) break;

  }

  /* We have left the read loop. If we haven't run into the end of
   * the file, the program is too large to fit in the TPA. Exit
   * with a complaint.
   */

  if( ReadStatus == 0 ) {
    bdos( bdos_setdmaaddress_c, (unsigned int)dmaaddress );
    return 1; 
  }

  /* Form a pointer to the executable header so that we can validate
   * it and extract the entry point address.
   */

  exehdr = (cpm_exehdr_t *)( lpb->tpabase + 0x100 );

  /* Make certain it was built for the correct architecture.
   */

  if( exehdr->tag != config_archtag_c ) {
    return 4;
  }

  /* Make certain it was built to load at the right address.
   */

  if( exehdr->region != ( lpb->tpabase + 0x100 ) ) {
    return 5;
  }

  /* We read to the end of the file before running out of memory
   * space. This means we now know how large the program is. Fill
   * in the base page and exit with success.
   */

  lpb->basepage->tpabase = (void *)lpb->tpabase;
  lpb->basepage->tpatop = (void *)lpb->tpatop;
  lpb->basepage->textbase = (void *)( lpb->tpabase + 0x100 );
  lpb->basepage->textlen = NextSector - ( lpb->tpabase + 0x100 );
  lpb->basepage->database = lpb->basepage->textbase;
  lpb->basepage->datalen = 0;
  lpb->basepage->bssbase = lpb->basepage->textbase;
  lpb->basepage->bsslen = 0;
  lpb->basepage->freelen = lpb->tpatop - NextSector;
  lpb->basepage->bdos = (void *)bdos;
  lpb->basepage->entry = exehdr->entry;
  lpb->basepage->initialsp = lpb->basepage->tpatop;

  /* Figure out from which drive the program was loaded. Start by
   * looking at the FCB drive byte; if it's not 0, use it.
   */

  lpb->basepage->loadedfrom = lpb->fcb[ 0 ];
  if( lpb->basepage->loadedfrom == 0 ) {

    /* FCB says default drive; fetch it and add one to make drive # */

    lpb->basepage->loadedfrom = bdos( bdos_getcurrentdisk_c, 0 ) + 1;
  }

  bdos( bdos_setdmaaddress_c, (unsigned int)dmaaddress );

  return 0;
}

/***
 *
 * initexc
 *
 *	This routine is called at system initialization time to set up
 *	the exception vectors. I'm not quite certain how this is
 *	supposed to work.
 *
 *	It looks like there's a set of user exception handlers kept
 *	in a list in the BDOS. When an exception occurs, the service
 *	routine examines that list to determine how it should be
 *	handled. A user program can set up a handler to be called
 *	when the exception occurs. The service routine examines the
 *	entry for the exception in the BDOS table. If a user exception
 *	handler is defined, it is invoked.
 *
 *	This routine is expected to do two things:
 *
 *	- Initialize (or re-initialize) the hardware exception vectors.
 *
 *	- Trim user exception vectors that lie outside the current TPA;
 *	  that is, exceptions that have not been defined by a TSR.
 *
 *	Since I'm currently ignoring exceptions this routine currently
 *	does nothing.
 *
 * revisions:
 *
 *	2005-03-05 rli: original version.
 *
 * formal parameters:
 *
 *	- parm: A pointer to the BDOS exception table.
 *
 * informal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	none.
 *
 ***/

void initexc( void *parm )
{
}

/***
 *
 * load68k
 *
 *	This routine is called to load a program of a type that is not
 *	listed in load_tbl, below. What this means is that the user
 *	has entered the entire file name (with extension) as the
 *	command and the file exists. The CCP has successfully opened
 *	the file, but cannot find the appropriate loader in load_tbl.
 *	So it punts by calling this routine.
 *
 *	I'm not terribly interested in handling unknown executable
 *	formats at the moment, so we'll return an error.
 *
 * revisions:
 *
 *	2005-03-05 rli: original version.
 *
 * formal parameters:
 *
 *	none.
 *
 * informal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- 0, indicating a general program load error.
 *
 ***/

unsigned short int load68k( void )
{
  return 0;
}

/***
 *
 * load_tbl
 *
 *	This table serves two purposes:
 *
 *	- It informs the system about the recognized executable extensions
 *	- It tells the system how to load a file of each type (except
 *	  for .SUB files, which have special handling in the CCP).
 *
 *	When the CCP is searching for a command file, it tries each of
 *	the extensions listed here, in order, for both the current user
 *	and user zero. If one of the files can be opened, the handler
 *	is called to load that file (except for .SUB files, which use
 *	special handline in the CCP).
 *
 *	Although .SUB files are specially recognized by the CCP, that
 *	type needs to be listed here if the CCP is to automatically
 *	search for that extension. If the type is not listed, the
 *	escape hatch noted above for load68k can be used by explicitly
 *	specifying the .SUB type (in this case, the special handling
 *	should get invoked and load68k will not be called).
 *
 * revisions:
 *
 *	2005-03-05 rli: original version.
 *
 *	2010-08-16 rli: changed recognized extension to .ARM. 
 *
 *	2013-05-27 rli: rennamed bios_loadarm to glue_loadarm.
 *
 *	2013-05-30 rli: switch to a platform-specific constant to
 *	  supply the extension for .COM-style executables. Rename
 *	  glue_loadarm to glue_loadcom.
 *
 *	2013-06-04 rli: platform_comextension_c changed to
 *	  config_comfileextension_c.
 *
 ***/

struct _filetyps
{
	signed char *typ;
	unsigned short int (*loader) ( void );
	signed char user_c;
	signed char user_0;
}
load_tbl[] = {
   { config_comfileextension_c, glue_loadcom, 0, 0 },
   { "\0", 0, 0, 0 }
};

#ifndef config_hastraphndl_c

/***
 *
 * traphndl
 *
 *	The BDOS initializes one of the vectors to this function,
 *	expecting it to handle traps. Since we're not using traps
 *	to get to the system, we don't need to do anything with
 *	it, but we do have to satisfy the linker.
 *
 * revisions:
 *
 *	2010-08-01 rli: original version.
 *
 *	2013-04-29 rli: return NULL to keep the compiler happy.
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

unsigned char *traphndl( void )
{
  return (void *)0;
}
#endif

