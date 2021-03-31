/******************************************************************
 *
 * file: glue.c
 *
 *	This file does initialization necessary before launching into
 *	a DRI-written utility such as PIP, STAT, or ED. It also includes
 *	glue functions expected by those programs.
 *
 *	Initialiation includes:
 *
 *	- Clearing the .bss section and adjusting the size of the free
 *	  memory space if necessary.
 *
 *	- Squirreling away a pointer to the basepage somewhere that
 *	  the program find it.
 *
 * revisions:
 *
 *	2010-08-20 rli: original version.
 *
 ******************************************************************/

/*****************
 *
 *	INCLUDES
 *
 *****************/

#include "cpm.h"

/*****************
 *
 * 	GLOBALS
 *
 *****************/

/***
 *
 * _base
 *
 *	Holds a pointer to the CP/M base page.
 *
 * revisions:
 *
 *	2010-08-20 rli: original version.
 *
 ***/

cpm_basepage_t *_base;

/***
 *
 * glue_brk
 *
 *	This variable keeps track of the next free address within
 *	the TPA. It is advanced when sbrk allocates space.
 *
 * revisions:
 *
 *	2010-08-20 rli: original version.
 *
 ***/

unsigned int glue_brk;

/*****************
 *
 *	PROTOTYPES
 *
 *****************/

void _main( void ); /* DRI entry point */
unsigned int __BDOS( unsigned short int func, unsigned int parm );

/*****************
 *
 *	CODE
 *
 *****************/

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
 *	2010-08-20 rli: version for apps.
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
    __BDOS( 2, Digit[ Victim >> 28 ] ) ;
    Victim = Victim << 4;
  }
}

/***
 *
 * glue_outzstring
 *
 *	displays a null-terminated string.
 *
 * revisions:
 *
 *	2010-08-21 rli: original version.
 *
 * formal parameters:
 *
 *	- victim: pointer to the string.
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

void glue_outzstring( char *victim )
{
  while( *victim ) {
    __BDOS( 2, *victim++ );
  }
}

/***
 *
 * glue_entry
 *
 *	This is the C entry point for the program; the executable header
 *	branches here. It performs initialization required before the DRI 
 *	code can be entered.
 *
 * revisions:
 *
 *	2010-08-20 rli: initial version.
 *
 *	2010-08-20 rli: Clear .bss. Ensure size of free memory is
 *	  correct; the .bss section is generally omitted by objcopy.
 *	  Initialize the base address of free space.
 *
 * formal parameters:
 *
 *	- basepage: Supplies a pointer to the CP/M base page.
 *
 * informal parameters:
 *
 *	- _base: Receives a pointer to the CP/M base page.
 *
 *	- __bss_start__: The starting address of the .bss section. This
 *	  is defined by the linker.
 *
 *	- __bss_end__: The ending address of the .bss section. This is
 *	  defined by the linker.
 *
 *	- __end__: The ending address of the image. This is defined by
 *	  the linker.
 *
 *	- glue_brk: Receives the starting address of free space.
 *
 * return value:
 *
 *	none.
 *
 ****/

void glue_entry( cpm_basepage_t *basepage )
{
  unsigned char *ThisByte;

  extern unsigned char __bss_start__;
  extern unsigned char __bss_end__;
  extern unsigned char __end__;

  unsigned int FreeLength;
 
  /* clear the .bss section.
   */

  for( ThisByte = &__bss_start__; ThisByte < &__bss_end__; ThisByte++ )
    *ThisByte = 0;

  /* save a pointer to the basepage so the program can find it.
   */
 
  _base = basepage;

  /* make certain the .bss section is not considered to be part of the
   * free space after the image.
   */

  FreeLength = (unsigned int)basepage->tpatop - (unsigned int)&__end__;
  if( FreeLength < basepage->freelen ) basepage->freelen = FreeLength;

  /* Figure out where the free space starts.
   */

  glue_brk = (unsigned int)basepage->tpatop - basepage->freelen;

  /* enter the program. 
   */

  _main();
}

/***
 *
 * __BDOS
 *
 *	Calls the BDOS.
 *
 * revisions:
 *
 *	2010-08-20 rli: original version.
 *
 * formal paramaters:
 *
 *	- func: Indicates which function is to be performed.
 * 
 *	- parm: Supplies a parameter for that function.
 *
 * informal parameters:
 *
 *	- _base: Holds a pointer to the base page.
 *
 * return value:
 *
 *	- The value returned by BDOS is passed along.
 *
 ***/

unsigned int __BDOS( unsigned short int func, unsigned int parm )
{
  return _base->bdos( func, parm );
} 

/***
 *
 * _exit
 *
 *	Performs a warm boot.
 *
 * revisions:
 *
 *	2010-08-20 rli: original version.
 *
 * formal parameters:
 *
 *	- status: Program exit status. This is not used.
 *
 * informal parameters:
 *
 *	- _base: Holds a pointer to the base page.
 *
 * return value:
 *
 *	This function does not return.
 *
 ***/

void _exit( unsigned int status )
{
  _base->bdos( 0, 0 );
}

/***
 *
 * memcpy
 *
 *	Copies a chunk o' memory.
 *
 * revisions:
 *
 *	2010-08-20 rli: original version.
 *
 * formal parameters:
 *
 *	- to: Supplies a pointer to the buffer that is to receive the
 *	  data.
 *
 *	- from: Supplies a pointer to the buffer that is to supply the
 *	  data.
 *
 *	- size: Supplies the size, in bytes, of the chunk that is to
 *	  be copied.
 *
 * informal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- A copy of to is returned.
 *
 * notes:
 *
 *	- regions are assumed to not overlap.
 *
 ***/

char *memcpy( char *to, char *from, unsigned int size )
{
  while( size-- ) {
    *to++ = *from++;
  }
}

/***
 *
 * sbrk
 *
 *	Allocates memory. The base address of free space is advance by
 *	the specified number of bytes, if enough space remains.
 *
 *	NOTE:	This function does not insure that allocated space
 *		does not collide with the stack!!
 *
 * revisions:
 *
 *	2010-08-20 rli: original version.
 *
 *	2010-08-21 rli: round allocation size up to longwords.
 *
 * formal parameters:
 *
 *	- nbytes: The size, in bytes, of the region to be allocated.
 *
 * informal parameters:
 *
 *	- glue_brk: The base address of the free space.
 *
 *	- _base: A pointer to the CP/M base page. This tells us where
 *	  the top of the TPA is.
 *
 * return value:
 *
 *	- -1: Failure. There's not enough memory to support the
 *	  allocation.
 *
 *	- else: The base address of the allocated region.
 *
 ***/

void *sbrk( unsigned int nbytes )
{
  unsigned int oldbrk;

  nbytes = ( nbytes + 3 ) & ~3;

  if( ( glue_brk + nbytes ) > (unsigned int)_base->tpatop ) 
    return (void *)-1;

  oldbrk = glue_brk;
  glue_brk += nbytes;
  return (void *)oldbrk;
}

