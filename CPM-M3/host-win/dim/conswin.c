/*********************************************************************
 *
 * file: conswin.c
 *
 *	This is a minimal console driver that allows the BDOS to display
 *	characters to stdout.
 *
 * revisions:
 *
 *	2011-03-09 rli: liberated consuart2 from arm-hawk and hacked
 *	  over.
 *
 *	2011-06-25 rli: initialize API list in driver descriptor.
 *
 *	2013-05-01 rli: liberated consunix and worked it over.
 *
 *	2013-06-16 rli: include more stuff for new source layout.
 *
 ********************************************************************/

/********************
 *
 *	INCLUDES
 *
 ********************/

#include "config.h"
#include "bios.h"
#include "modubios.h"
#include <stdio.h>
#include <conio.h>

/*******************
 *
 *	PROTOTYPES
 *
 *******************/

void conswin_boot( void  );
char conswin_conin( void );
void conswin_conout( char victim );
unsigned short int conswin_conist( void );
unsigned short int conswin_conost( void );

/*******************
 *
 *	GLOBALS
 *
 *******************/

/***
 *
 * conswin
 *
 *	This structure allows the modular BIOS to locate the services
 *	provided by this driver.
 *
 * revisions:
 *
 *	2010-08-10 rli: tag and version.
 *
 *	2011-06-25 rli: initialize API list header.
 *
 *	2013-05-01 rli: liberated from consunix and hacked over.
 *
 ***/

modubios_cons_t conswin = {
  0x006e6977,		/* "win" */
  0x20100810,
  conswin_boot,	/* cboot */
  conswin_boot,	/* wboot */
  conswin_boot,	/* close */
  conswin_conin,
  conswin_conout,
  conswin_conist,
  conswin_conost,
  0
};

/**************************
 *
 *	CODE
 *
 **************************/

/***
 *
 * conswin_boot
 *
 *	This function is used for the cold boot, warm boot, and close
 *	services. We don't need to do anything for these.
 *
 * revisions:
 *
 *	2010-08-08 rli: original version.
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

void conswin_boot( void )
{
}

/***
 *
 * conswin_conist
 *
 *	This function examines the UART to determine whether a character
 *	has been received.
 *
 * revisions:
 *
 *	2011-03-09 rli: this console doesn't support input; claim no
 *	  characters are available.
 *
 *	2013-05-01 rli: liberated from consunix and hacked over.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- 0 is always returned.
 *
 ***/

unsigned short int conswin_conist( void )
{
  if( _kbhit() != 0 ) return 255;
  return 0;
}

/***
 *
 * conswin_conost
 *
 *	This function examines the UART to determine whether a character
 *	can be sent. This won't be used; just claim it's always ready.
 *
 * revisions:
 *
 *	2011-03-09 rli: claim it's always ready.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- 255 is always returned.
 *
 ***/

unsigned short int conswin_conost( void )
{
  return 255;
}

/***
 *
 * conswin_conin
 *
 *	This function waits for a character to arrive from the console,
 *	then returns that character. We don't support input, so just
 *	pretend a ^C was typed.
 *
 * revisions:
 *
 *	2011-03-09 rli: always ^C.
 *
 *	2011-09-30 rli: switch to getchar instead of just returning ^C;
 *	  I'm hoping this will let me use BDOS(10).
 *
 *	2013-05-01 rli: liberated from consunix and switched to _getch.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- ^C is always returned.
 *
 ***/

char conswin_conin( void )
{
  return _getch();
}

/***
 *
 * conswin_conout
 *
 *	This function waits for the console interface to be ready to
 *	send a character, then sends a character.
 *
 * revisions:
 *
 *	2011-03-09 rli: win version.
 *
 * parameters:
 *
 *	- victim: the character to be sent.
 *
 * return value:
 *
 *	none.
 *
 ***/

void conswin_conout( char victim )
{
  putchar( victim );
  fflush( stdout );
}

