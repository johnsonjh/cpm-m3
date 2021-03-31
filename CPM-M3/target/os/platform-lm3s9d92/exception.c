/************************************************************************
 *
 * file: exception.c
 *
 *	This file deals with exceptions. It contains a high-level
 *	exception handler that prints information about the exception
 *	that occurred and resets the processor. It relies on a low-level
 *	handler to build a structure holding the processor state.
 *
 *	This file also contains the program's entry point, entry. The
 *	bootstrap calls that function after it copies the image to RAM
 *	and clears the .bss section.
 *
 * revisions:
 *
 *	2013-12-22 rli: original attempt.
 *
 ***********************************************************************/

/************************
 *
 *	INCLUDES
 *
 ************************/

#include "cpm.h"
#include "bios.h"
#include "glue.h"

#include "config.h"
#include "modubios.h"
#include "platform.h"

#include "cpureg.h"


/************************
 *
 *	GLOBALS
 *
 ************************/

extern unsigned char vectors[];

/************************
 *
 *	TYPES
 *
 ************************/

/***
 *
 * exception_t
 *
 *	A structure of this type is created when an exception occurs.
 *	Part of it is filled in by the hardware, the remainder by the
 *	low-level exception handler. A pointer to the structure is
 *	passed to the high-level exception handler.
 *
 * revisions:
 *
 *	2013-12-22 rli: first attempt.
 *
 ***/

typedef struct exception_s {

  /* this portion created by the low-level exception handler */

  unsigned int r4;
  unsigned int r5;
  unsigned int r6;
  unsigned int r7;
  unsigned int r8;
  unsigned int r9;
  unsigned int r10;
  unsigned int r11;

  /* this portion created by hardware. */

  unsigned int r0;
  unsigned int r1;
  unsigned int r2;
  unsigned int r3;
  unsigned int r12;
  unsigned int lr;
  unsigned int pc;
  unsigned int psr;

} exception_t;

/**************************
 *
 *	CODE
 *
 **************************/

/***
 *
 * exception
 *
 *	This function is called by the low-level exception handler
 *	when an exception occurred. It is passed a pointer to a
 *	structure holding the majority of the processor state.
 *
 *	The pointer itself reflects the value in the stack pointer. If
 *	the stack pointer needed alignment, bit 9 in the psr will be
 *	set.
 *
 * revisions:
 *
 *	2013-12-22 rli: original attempt.
 *
 * parameters:
 *
 *	- oops: Supplies a pointer to a structure that describes the
 *	  processor state at the time of the exception.
 *
 * return value:
 *
 *	this function does not return.
 *
 * notes:
 *
 *	- The primary reason for the heading and trailing lines of
 *	  hyphens is to ensure that the display of the processor state
 *	  has made it out of the UART before we reset the processor.
 *
 ***/

void exception( exception_t *oops )
{
  unsigned int sp;
  unsigned int excno;

  /* figure out the original value of the stack pointer.
   */

  sp = (unsigned int)oops;
  sp = sp + sizeof( exception_t );
  if( ( oops->psr & 0x200 ) != 0 ) {
    sp = sp + 4;
  }

  /* get the currently active vector number.
   */

  excno = cpureg_intctrl_g & cpureg_intctrl_vecact_m;

  /* display the processor state.
   */

  glue_outzstring( "\r\n\n----------------------------" );
  glue_outzstring( "\r\nexception " );
  glue_outhex( excno );
  glue_outzstring( " at " );
  glue_outhex( oops->pc );
  glue_outzstring( "\r\n\nregisters:\r\n\n" );

  glue_outhex( oops->r0 );
  glue_outzstring( " " );
  glue_outhex( oops->r1 );
  glue_outzstring( " " );
  glue_outhex( oops->r2 );
  glue_outzstring( " " );
  glue_outhex( oops->r3 );
  glue_outzstring( " " );
  glue_outhex( oops->r4 );
  glue_outzstring( " " );
  glue_outhex( oops->r5 );
  glue_outzstring( " " );
  glue_outhex( oops->r6 );
  glue_outzstring( " " );
  glue_outhex( oops->r7 );
  glue_outzstring( "\r\n" );

  glue_outhex( oops->r8 );
  glue_outzstring( " " );
  glue_outhex( oops->r9 );
  glue_outzstring( " " );
  glue_outhex( oops->r10 );
  glue_outzstring( " " );
  glue_outhex( oops->r11 );
  glue_outzstring( " " );
  glue_outhex( oops->r12 );
  glue_outzstring( " " );
  glue_outhex( sp );
  glue_outzstring( " " );
  glue_outhex( oops->lr );
  glue_outzstring( " " );
  glue_outhex( oops->pc );
  glue_outzstring( "\r\n\n----------------------------\r\n\n" );

  cpureg_apint_g = cpureg_apint_sysresreq_c;

  while( 1 ) {
  }

}

/***
 *
 * entry
 *
 *	This is the program entry point. The bootstrap enters here after
 *	it copies the system to RAM and clears the .bss section.
 *
 *	We relocate the vector table to refer to the one in RAM and 
 *	enter the bios at bios_cboot.
 *
 * revisions:
 *
 *	2013-12-22 rli: original attempt.
 *
 *	2014-01-01 rli: initialize the MRT.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	none.
 *
 ***/

void entry( void )
{
  /* point the vector table at the RAM variant.
   */

  cpureg_vtor_g = (unsigned int)vectors;

  /* initialize the memory region table; the TPA begins at the 
   * start of RAM and ends at the vectors.
   */

  platform_mrt.regions[ 0 ].base = (void *)cpureg_ram_g;
  platform_mrt.regions[ 0 ].length = 
    ( (unsigned int)vectors ) - ( (unsigned int)cpureg_ram_g );

  /* enter the BIOS.
   */

  bios_cboot();
}

