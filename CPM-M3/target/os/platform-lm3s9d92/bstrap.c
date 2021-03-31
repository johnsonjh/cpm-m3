/********************************************************************
 *
 * file: bstrap.c
 *
 *	This is the bootstrap; it contains the reset entry point and
 *	exception handlers for use during boot. The bootstrap copies
 *	the system to its final location in RAM and jumps to it.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 ********************************************************************/

/********************
 *
 *	GLOBALS created by the linker
 *
 ********************/

extern char bstrap_copy_from;
extern char bstrap_copy_to;
extern char bstrap_copy_size;
extern char bstrap_zero_to;
extern char bstrap_zero_size;

/********************
 *
 *	PROTOTYPES
 *
 ********************/

void entry( void );

/********************
 *
 *	CODE
 *
 ********************/

/***
 *
 * bstrap
 *
 *	This function is the reset entry point. It copies the system to
 *	RAM, clears the .bss section, and jumps to the system entry
 *	point, entry.
 *
 *	It relies on a handful of symbols created by the linker:
 *
 *	- bstrap_copy_from: The load address of the image; i.e., its
 *	  location in flash.
 *
 *	- bstrap_copy_to: The execution address of the image; i.e., its
 *	  location in RAM.
 *
 *	- bstrap_copy_size: The number of bytes to be copied.
 *
 *	- bstrap_zero_to: The address of the first location in RAM to
 *	  be cleared.
 *
 *	- bstrap_zero_size: The size, in bytes, of the region to be
 *	  cleared.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	this function does not return.
 *
 ***/

void bstrap( void )
{
  char *from;
  char *to;
  unsigned int size;

  /* copy the image to RAM.
   */

  from = &bstrap_copy_from;
  to = &bstrap_copy_to;
  size = (unsigned int)&bstrap_copy_size;
  while( size != 0 ) {
    *to++ = *from++;
    size--;
  }

  /* clear the .bss section.
   */

  to = &bstrap_zero_to;
  size = (unsigned int)&bstrap_zero_size;
  while( size != 0 ) {
    *to++ = 0;
    size--;
  }

  /* enter the image.
   */

  entry();

  /* the image should not return. if it does, hang.
   */

  while( 1 ) {
  }

}

/***
 *
 * bstrap_exception
 *
 *	This is a catch-all exception routine for use in the bootstrap.
 *	The theory is that the running system will build a functional
 *	vector table elsewhere.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
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

void bstrap_exception( void )
{
  while( 1 ) {
  }
}

