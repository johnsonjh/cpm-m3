/*******************************************************************
 *
 * file: consnull.c
 *
 *	This file contains a null console device driver for the modular
 *	BIOS.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version.
 *
 *	2010-08-08 rli: add a close routine.
 *
 *	2011-06-25 rli: init API list pointer in driver descriptor.
 *
 *	2013-06-08 rli: include config.h so the host tools can #define
 *	  away gcc packing.
 *
 ******************************************************************/

/*****************************
 *
 *	INCLUDES
 *
 *****************************/

#include "config.h"
#include "bios.h"
#include "modubios.h"

/*****************************
 *
 *	PROTOTYPES
 *
 *****************************/

void consnull_cboot( void );
void consnull_wboot( void );
void consnull_close( void );
char consnull_conin( void );
void consnull_conout( char victim );
unsigned short int consnull_conist( void );
unsigned short int consnull_conost( void );

/*****************************
 *
 *	GLOBALS
 *
 *****************************/

/***
 *
 * consnull
 *
 *	This structure describes the null device to the modular bios. It
 *	supplies the BIOS with pointers to each of the services offered
 *	by the device.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version.
 *
 *	2010-08-10 rli: tag and version.
 *
 *	2011-06-25 rli: initialize API list pointer.
 *
 ***/

modubios_cons_t consnull = {
  0x6c6c756e,		/* "null" */
  0x20100810,
  consnull_cboot,
  consnull_wboot,
  consnull_close,
  consnull_conin,
  consnull_conout,
  consnull_conist,
  consnull_conost,
  0
};

/*****************
 *
 *	CODE
 *
 *****************/

/***
 *
 * consnull_cboot
 *
 *	This function is called during cold boot, before much
 *	initialization has been performed. It is intended to prepare
 *	the port for use.
 *
 *	Since the console is used to display the copyright message
 *	before the BDOS is initialized, this function cannot assume
 *	BDOS services are available. A warm boot is performed during
 *	the cold boot sequence after the BDOS has been initialized;
 *	actions that need the BDOS can be tucked there.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version.
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

void consnull_cboot( void )
{
}

/***
 *
 * consnull_wboot
 *
 *	This function is called during a warm boot. It is intended to
 *	allow the device to handle things that need to be done at that
 *	time; for example, a list device using an automagic printer
 *	switch might flush its buffer and release ownership of the
 *	switch.
 *
 * 	A warm boot is performed during the cold boot sequence after the
 *	BDOS has been initialized.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version.
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

void consnull_wboot( void )
{
}

/***
 *
 * consnull_conin
 *
 *	This function waits for a character to arrive from the device
 *	and returns that character.
 *
 *	Reader devices should return ^Z (26 decimal) to report end of
 *	file.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- The received character.
 *
 * notes:
 *
 *	- The null device is always at end of file. It always returns
 *	  ^Z.
 *
 ***/

char consnull_conin( void )
{
  return 26;
}

/***
 *
 * consnull_conout
 *
 * 	This function sends a character to the device. If the device
 *	is not ready to accept a character, this function waits for
 *	it to become ready.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version.
 *
 * parameters:
 *
 *	- victim: supplies the character that is to be sent.
 *
 * return value:
 *
 *	none.
 *
 ***/

void consnull_conout( char victim )
{
}

/***
 *
 * consnull_conist
 *
 *	This function examines the device to see if a character has
 *	been received.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- 0: A character has not been received from the device.
 *
 *	- 255: A character is available from the device.
 *
 * notes:
 *
 *	- The null device always claims that it has not received a
 *	  character.
 *
 ***/

unsigned short int consnull_conist( void )
{
  return 0;
}

/***
 *
 * consnull_conost
 *
 *	This function examines the device to see whether it is ready to
 *	accept a device. This is used in the BIOS API to support print
 *	spooling.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- 0: The device is not ready to accept a character.
 *
 *	- 255: The device can accept a character.
 *
 * notes:
 *
 *	- The null device is always ready to accept a character.
 *
 ***/

unsigned short int consnull_conost( void )
{
  return 255;
}

/***
 *
 * consnull_close
 *
 *	This function is called when the IOBYTE is changed away from
 *	this device. It gives the driver an opportunity to do some
 *	finalization.
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

void consnull_close( void )
{
}

