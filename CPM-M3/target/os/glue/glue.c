/************************************************************************
 *
 * file: glue.c
 *
 *	This file contains routines that sit between the system-specific
 *	stuff and the operating system which are not directly referred
 *	to by the operatin system. Simple systems won't need to change
 *	this stuff if they are willing to live within its limitations.
 *
 * revisions:
 *
 *	2013-05-27 rli: split old arch.c into calledbydri.c and glue.c
 *	  Stuff that lives here begins with glue_, providing rigidly
 *	  defined boundaries on the doubt and uncertainty.
 *
 *	2013-05-30 rli: rename glue_loadarm to glue_loadcom.
 *
 *	2013-06-09 rli: include config.h so the host tools can #define
 *	  away gcc packing.
 *
 ************************************************************************/ 

/******************
 *
 *	INCLUDES
 *
 ******************/

#include "config.h"
#include "cpm.h"
#include "arch.h"
#include "bios.h"
#include "glue.h"

/***********************
 *
 *	PROTOTYPES
 *
 ***********************/

/***
 *
 * glue_outzstring
 *
 *	This routine displays a null-terminated string using only the
 *	BIOS CONOUT service. It is used for diagnostics and to display
 *	an error message on an unhandled exception.
 *
 * revisions:
 *
 *	2005-03-07 rli: Added these comments.
 *
 *	2013-05-29 rli: renamed.
 *
 * formal parameters:
 *
 *	- Victim: A pointer to the string to be displayed.
 *
 * informal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	none.
 *
 * side effects:
 *
 *	none.
 *
 ***/

void glue_outzstring( unsigned char *Victim )
{
  while( *Victim ) {
    bios_conout( *Victim );
    Victim++;
  }
}

/***
 *
 * glue_outhex
 *
 *	This diagnostic function is used to display a hexadecimal
 *	integer, if necessary.
 *
 * revisions:
 *
 *	2005-03-07 rli: Added these comments.
 *
 *	2013-05-29 rli: renamed.
 *
 * formal parameters:
 *
 *	- Victim: The value to be displayed.
 *
 * informal parameters:
 *
 *	none
 *
 * return value:
 *
 *	none
 *
 * side effects:
 *
 *	none
 *
 ***/

void glue_outhex( unsigned int Victim )
{
  static unsigned char Digit[] = "0123456789abcdef";
  int Count;

  /* I'm sure you know the drill; a nybble at a time beginning
   * with the most-significant.
   */

  for( Count = 0; Count < 8; Count++ ) {
    bios_conout( Digit[ Victim >> 28 ] );
    Victim = Victim << 4;
  }
}

/***
 *
 * glue_callbios
 *
 *	This routine implements BDOS function 50, DIRECT BIOS CALL,
 *	which allows a transient program to call a BIOS function.
 *
 * revisions:
 *
 *	2005-03-05 rli: original version.
 *
 *	2010-08-02 rli: genericized structure names.
 *
 *	2010-08-03 rli: use cold and warm boot entries.
 *
 *	2010-08-22 rli: service 23, return a description of the BIOS.
 *
 *	2010-08-28 rli: switch to CP/M-68K layout of bpb so that I can
 *	  be lazy when manual writing time comes. return value stretches
 *	  to a 32-bit int so that pointers will fit.
 *
 *	2013-04-29 rli: deleted a couple of unreachable breaks.
 *
 *	2013-05-03 rli: use bios function number constants from cpm.h.
 *
 *	2013-05-29 rli: renamed.
 *
 * formal parameters:
 *
 *	- BiosParameterBlock: A pointer to the cpm_bpb_t that describes the 
 *	  function to be called.
 *
 * informal parameters:
 *
 *	- the bios function number constants.
 *
 * return value:
 *
 *	- If the BIOS function returns something, its return value is
 *	  passed along.
 *
 *	- 0 is returned for functions that don't return something.
 *
 * side effects:
 *
 *	Depends upon the BIOS function called.
 *
 ***/

unsigned int glue_callbios( cpm_bpb_t *BiosParameterBlock )
{
  switch( BiosParameterBlock->func ) {
    case bios_coldboot_c: bios_cboot();
    case bios_warmboot_c: bios_wboot();
    case bios_constat_c:  return (unsigned int)bios_const();
    case bios_conin_c:    return (unsigned int)bios_conin();
    case bios_conout_c:   bios_conout( (unsigned char)BiosParameterBlock->p1 );
                          return 0;
    case bios_lstout_c:   bios_list( (unsigned char)BiosParameterBlock->p1 );
                          return 0;
    case bios_punout_c:   bios_punch( (unsigned char)BiosParameterBlock->p1 );
                          return 0;
    case bios_rdrin_c:    return (unsigned int)bios_reader();
    case bios_home_c:     bios_home();
                          return 0;
    case bios_seldsk_c:   return (unsigned int)bios_seldsk(
                           (unsigned char)BiosParameterBlock->p1,
                           (unsigned char)BiosParameterBlock->p2 );
    case bios_settrk_c:   bios_settrk( 
                            (unsigned short int)BiosParameterBlock->p1 );
                          return 0;
    case bios_setsec_c:   bios_setsec( 
                            (unsigned short int)BiosParameterBlock->p1 );
                          return 0;
    case bios_setdma_c:   bios_setdma( (void *)BiosParameterBlock->p1 );
                          return 0;
    case bios_read_c:     return (unsigned int)bios_read();
    case bios_write_c:    return (unsigned int)bios_write(
                            (unsigned short int)BiosParameterBlock->p1 );
    case bios_liststat_c: return (unsigned int)bios_listst();
    case bios_sectrn_c:   return (unsigned int)bios_sectran(
                            (unsigned short int)BiosParameterBlock->p1,
                            (unsigned short int *)BiosParameterBlock->p2 );
    case bios_getseg_c:   return (unsigned int)bios_getmrt();
    case bios_getiob_c:   return (unsigned int)bios_getiobyte();
    case bios_setiob_c:   bios_setiobyte( 
                            (unsigned short int)BiosParameterBlock->p1 );
                          return 0;
    case bios_bflush_c:   bios_flush();
                          return 0;
    case bios_setvec_c:   return 0;
    case bios_getbios_c:  return (unsigned int)bios_getbios();
    default:              return 0;
  }
}

/***
 *
 * glue_enterprogram
 *
 *	BDOS FUNCTION 64 ENTER PROGRAM
 *
 *	This routine enters a program that has been loaded via
 *	BDOS function 59, LOAD PROGRAM. Given the address of the
 *	base page, it initializes the stack and enters the program.
 *
 *	If the program returns, the system is rebooted.
 *
 * revisions:
 *
 *	2005-03-05 rli: original attempt.
 *
 *	2010-08-01 rli: initialization of stack pointer moved to
 *	  boot_enter.
 *
 *	2010-08-02 rli: genericized structure names.
 *
 *	2013-05-29 rli: renamed. renamed boot_enter to arch_enter. Add
 *	  a return statement so picky compilers won't complain (we
 *	  shouldn't get to it, but we're declared to return an
 *	  unsigned int so that all paths in bdos() return something).
 *
 * formal parameters:
 *
 *	- BasePage: Pointer to the base page describing the program to be
 *	  entered.
 *
 * informal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	this function does not return.
 *
 ***/

unsigned int glue_enterprogram( cpm_basepage_t *basepage )
{
  arch_enter( basepage, basepage->entry, basepage->initialsp );
  return 0;
}
   
/***
 *
 * glue_loadcom
 *
 *	This routine loads and starts a program with the .ARM extension.
 *	Such programs are in the same naive format as a CP/M-80 .COM
 *	file: a memory dump of the TPA.
 *
 *	The file is loaded into the current TPA using BDOS function 59.
 *	The portion of the base page not initialized by that function
 *	is filled in, and the program is entered using BDOS function 64,
 *	which is unique to this port.
 *
 * revisions:
 *
 *	2005-03-05 rli: original version.
 *
 *	2010-08-01 rli: entry moved to boot.
 *
 *	2010-08-02 rli: genericized structure names.
 *
 *	2010-08-16 rli: name changes.
 *
 *	2010-08-19 rli: base page rework for CP/M-68K and CP/M-8K
 *	  compatibility.
 *
 *	2010-08-21 rli: restore user number before entering program;
 *	  CCP may have changed it to search for executable in user 0.
 *
 *	2010-08-28 rli: use cpm_tpa_t with function 63.
 *
 *	2013-05-03 rli: use bdos function number constants from cpm.h.
 *
 *	2013-05-29 rli: renamed. exit via bios_cboot instead of
 *	  boot_enter if enterprogram returns.
 *
 *	2013-05-30 rli: rename to glue_loadcom.
 *
 * formal parameters:
 *
 *	none.
 *
 * informal parameters:
 *
 *	- user: The user number that was in effect before the CCP
 *	  started searching for the program. The CCP will try both
 *	  user 0 and the current user. The original user number
 *	  needs to be restored before the program is entered.
 *
 *	- cmdfcb: The FCB open on the command file.
 *
 *	- tail: The command tail.
 *
 *	- user: The original user number.
 *
 * return value:
 *
 *	- See pgmld
 *
 *	- This routine does not return if the load was successful.
 *
 * side effects:
 *
 *	With luck, the program is loaded and entered.
 *
 ***/

unsigned short int glue_loadcom( void )
{
  extern unsigned short int user;
  extern unsigned char cmdfcb[];
  extern unsigned char *tail;
  cpm_lpb_t lpb;
  cpm_tpa_t tpab;
  unsigned short int Status;
  unsigned short int fill_fcb( unsigned short int which, 
    cpm_fcb_t *fcb );
  unsigned char *src,*dst;

  /* We need to load the program into the current TPA. This means
   * we have to ask the BDOS for the TPA, since it can be changed
   * by a program (and the changes aren't passed on to the BIOS).
   */

  tpab.flags = 0;
  bdos( bdos_getsettpalimits_c, (unsigned int)&tpab );
  lpb.tpabase = (unsigned int)tpab.base;
  lpb.tpatop = (unsigned int)tpab.top;

  /* Load the program into the current TPA.
   */

  lpb.fcb = cmdfcb;
  Status = bdos( bdos_loadprogram_c, (unsigned int)&lpb );
  if( Status != 0 ) return Status;

  /* Copy the command tail over. The tail is ended by either a null
   * or an exclamation point.
   */

  src = tail;
  dst = (lpb.basepage->tail) + 1;
  do {
    *dst = *src;
    if( ( *src == 0 ) || ( *src == '!' ) ) break;
    dst++; src++;
  } while( 1 );
  lpb.basepage->tail[ 0 ] =
    (unsigned int)src - (unsigned int)tail;

  /* The program has been loaded. Now we need to fill in the remainder
   * of the base page.
   */

  fill_fcb( 1, &lpb.basepage->fcb1 );
  fill_fcb( 2, &lpb.basepage->fcb2 );

  /* Set the default DMA address, which traditionally points to the
   * portion of the base page containing the command tail.
   */

  bdos( bdos_setdmaaddress_c, (unsigned int)lpb.basepage->tail );

  /* The base page is set up. Restore the user number and Enter the 
   * program.
   */

  bdos( bdos_getsetusernumber_c, user );
  Status = bdos( bdos_enterprogram_c, (unsigned int)lpb.basepage );

  /* The BDOS does not expect this function to return if there is no
   * error.
   */

  bios_cboot();
}
