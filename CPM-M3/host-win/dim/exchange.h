/**************************************************************************
 *
 * file: exchange.h
 *
 *	This file contains declarations related to the CCP extensions that 
 *	allow files to be exchanged between the host file systems and disk
 *	image being manipulated.
 *
 * revisions:
 *
 *	2013-06-16 rli: original version.
 *
 *	2013-12-21 rli: added PUTSYS.
 *
 **************************************************************************/

#ifndef exchange_h_Included
#define exchange_h_Included

/*****************************
 *
 *	PROTOTYPES
 *
 *****************************/

void exchange_exit( void );
void exchange_export( void );
void exchange_flushdrives( void );
void exchange_import( void );
void exchange_putsys( void );
#endif
