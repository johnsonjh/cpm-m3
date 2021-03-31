/***************************************************************************
 *
 * file: ccphooks.c
 *
 *	This file contains the hooks used by dim to add commands to the CCP's 
 *	reportoire.
 *
 * revisions:
 *
 *	2013-06-16 rli: original version.
 *
 *	2013-12-21 rli: putsys.
 *
 ***************************************************************************/

/********************
 *
 *	INCLUDES
 *
 ********************/

#include <string.h>
#include <stdio.h>

#include "config.h"
#include "exchange.h"

/********************
 *
 *	TYPES
 *
 ********************/

/***
 *
 * ccphooks_cmdtable_t
 *
 *	This type provides a mapping between strings, command numbers, and 
 *	handling functions.
 *
 * revisions:
 *
 *	2013-06-16 rli: original version.
 *
 * fields:
 *
 *	- Command: Holds a pointer to the command name. This should be 
 *	  uppercase.
 *
 *	- Number: Holds the number of the command.
 *
 *	- Handler: Holds a pointer to the function that implements the command.
 *
 ***/

typedef struct ccphooks_cmdtable_s {
  signed char *Command;
  unsigned short int Number;
  void (*Handler)( void );
} ccphooks_cmdtable_t;

/**************************
 *
 *	GLOBALS
 *
 **************************/

/***
 *
 * ccphooks_cmdtable_g
 *
 *	This table holds a mapping between a command's name, its number, and 
 *	the function that handles it. When the CCP sees a command it doesn't 
 *	recognize, it calls platform_ccpdecode, which searches this table to 
 *	convert a command name into its number. Upon finding a command number 
 *	it doesn't recognize, the CCP calls platform_ccpexecute to execute 
 *	the command; that function searches this table for the command number 
 *	and calls the indicated function.
 *
 *	The table is ended by an entry in which the command name pointer is 
 *	NULL.
 *
 * revisions:
 *
 *	2013-06-16 rli: original version containing only EXIT.
 *
 *	2013-06-19 rli: IMPORT, EXPORT, and FlUSH.
 *
 *	2013-12-21 rli: PUTSYS.
 *
 ***/

ccphooks_cmdtable_t ccphooks_cmdtable_g[] = {
  { "EXIT", 255, exchange_exit },
  { "IMPORT", 254, exchange_import },
  { "EXPORT", 253, exchange_export },
  { "FLUSH", 252, exchange_flushdrives },
  { "PUTSYS", 251, exchange_putsys },
  { 0, 0, 0 }
};

/*******************************
 *
 *	CODE
 *
 *******************************/

/***
 *
 * platform_ccpdecode
 *
 *	The CCP calls this function when it sees a command it does not 
 *	recognize. This	functions searches the command table for the 
 *	associated command number.
 *
 * revisions:
 *
 *	2013-06-16 rli: original version.
 *
 * formal parameters:
 *
 *	- Command: Supplies a pointer to a string holding the command 
 *	  that the CCP does not recognize.
 *
 * informal parameters:
 *
 *	- ccphooks_cmdtable_g: Provides a mapping between commands, their 
 *	  numbers, and the functions that implement them.
 *
 * return value:
 *
 *	- 0: The command is not recognized.
 *
 *	- else: The number that corresponds to the specified command. If this 
 *	  is not a number that the CCP recognizes, it will call 
 *	  platform_ccpexecute to execute the command.
 *
 ***/

unsigned short int platform_ccpdecode( signed char *Command )
{
  ccphooks_cmdtable_t *ThisCommand;

  ThisCommand = ccphooks_cmdtable_g;
  while( ThisCommand->Command != (void *)0 ) {
    if( strcmp( Command, ThisCommand->Command ) == 0 ) 
      return ThisCommand->Number;
    ThisCommand++;
  }

  return 0;
}

/***
 *
 * platform_ccpexecute
 *
 *	The CCP calls this function to execute a command whose number it 
 *	does not recognize. It searches the command table to find the 
 *	associated handler.
 *
 * revisions:
 *
 *	2013-06-16 rli: original version.
 *
 * formal parameters:
 *
 *	- CmdNo: The number of the command that is to be executed.
 *
 * informal parameters:
 *
 *	- ccphooks_cmdtable_g: Provides a mapping between a command's name, 
 *	  number, and handling function.
 *
 * return value:
 *
 *	- 0: The command was not recognized.
 *
 *	- else: The command has been executed.
 *
 ***/

unsigned int platform_ccpexecute( unsigned short int CmdNo )
{
  ccphooks_cmdtable_t *ThisCommand;

  ThisCommand = ccphooks_cmdtable_g;
  while( ThisCommand->Command != (void *)0 ) {
    if( ThisCommand->Number == CmdNo ) {
      ThisCommand->Handler();
      return 1;
    }
    ThisCommand++;
  }
  return 0;

}
