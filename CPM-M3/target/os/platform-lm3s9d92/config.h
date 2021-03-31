/*************************************************************************
 *
 * file: config.h
 *
 *	This file contains configuration constants and prototypes for
 *	platform-specific functions that do not rely on the modubios
 *	definitions. The primary goal is to allow this file to be
 *	included by the CCP without having to also pull in other
 *	headers.
 *
 * revisions:
 *
 *	2013-06-04 rli: original version.
 *
 *	2013-12-21 rli: udpates for Cortex-M3 port.
 *
 *	2014-12-29 rli: add __DATE__ to banner.
 *
 ************************************************************************/

#ifndef config_h_included
#define config_h_included

/*******************************
 *
 *	CONSTANTS
 *
 *******************************/

/***
 *
 * config_comfileextension_c
 *
 *	This constant supplies a string containing the file extension
 *	that is to be used for simple .COM-style executables. 
 *
 *	I suspect this needs to be exactly three characters long; padded
 *	by spaces.
 *
 * revisions:
 *
 *	2013-06-04 rli: moved here from platform.h.
 *
 ***/

#define config_comfileextension_c "COM"

/***
 *
 * config_archtag_c
 *
 *	This constant is used as an architecture identifier in the
 *	modubios descriptor and, optionally, the simple .COM-style
 *	executables.
 *
 * revisions:
 *
 *	2013-06-04 rli: moved here from platform.h
 *
 *	2013-12-21 rli: "M3" for Cortex-M3.
 *
 ***/

#define config_archtag_c 0x0000334d /* "M3" */

/***
 *
 * config_banner_c
 *
 *	If this constant is defined, it supplies a string that is
 *	displayed after the Digital Research copyright message
 *	during cold boot.
 *
 * revisions:
 *
 *	2013-06-04 rli: moved here from platform.h
 *
 *	2013-12-21 rli: update for Cortex-M3 port.
 *
 ***/

#define config_banner_c "(Cortex-M3 port for EK-LM3S9D92 built " __DATE__ ")"

/***
 *
 * config_hasquiesce_c
 *
 *	If this constant is defined, the modubios warmboot function
 *	will call platform_quiesce to allow unused peripherals that
 *	may have been enabled by a user program to be shut off.
 *
 * revisions:
 *
 *	2013-06-04 rli: original version.
 *
 ***/

/*#define config_hasquiesce_c 1*/

/***
 *
 * config_hassetexc_c
 *
 *	If this constant is defined, modubios will call platfrom_setexc
 *	when a program wants to change an exception vector.
 *
 * revisions:
 *
 *	2013-06-04 rli: original version.
 *
 ***/

/*#define config_hassetexc_c 1*/

/***
 *
 * config_hasccphooks_c
 *
 *	If this symbol is defined, the CCP calls platform_ccpdecode
 *	and platform_ccpexecute to deal with commands that it does
 *	not recognize.
 *
 * revisions:
 *
 *	2013-06-05 rli: original version.
 *
 ***/

/*#define config_hasccphooks_c 1*/

/***
 *
 * config_trap2v_c
 *
 *	If this constant is defined, it is used as the trap 2 vector
 *	number instead of the hardcoded constant 34.
 *
 *	During cold boot, the BDOS calls bios_setexc in order to
 *	initialize trap 2. This allows the number of the trap2 vector
 *	to be controlled.
 *
 * revisions:
 *
 *	2013-06-05 rli: original version.
 *
 ***/
 
/*#define config_trap2v_c 0*/

/***
 *
 * config_hastraphndl_c
 *
 *	If this constant is defined, the trap 2 handler used by the
 *	BDOS, traphndl, calls platform_traphndl instead of doing
 *	nothing.
 *
 * revisions:
 *
 *	2013-06-05 rli: original version.
 *
 ***/

/*#define config_hastraphndl_c 1*/

/**************************
 *
 *	PROTOTYPES
 *
 **************************/

/***
 *
 * platform_quiesce
 *
 *	If config_hasquiesce_c is defined, the modubios warmboot
 *	function calls this function to allow the platform support code
 *	an opportunity to disable unused hardware that may have been
 *	started by a user program. The primary concern is things like
 *	Ethernet controllers, which tend to scribble on memory.
 *
 * revisions:
 *
 *	2013-06-04 rli: moved here from platfrom.h.
 *
 ***/

void platform_quiesce( void );

/***
 *
 * platform_setexc
 *
 *	If config_hassetexc_c is defined, modubios will call this
 *	function during cold boot as the BDOS initializes. The vector
 *	number passed in by the BDOS can be controlled via a #define
 *	in the BDOS.
 *
 *	There's a bit of a loose end just at the moment; the only
 *	time this is called by the BDOS is during cold boot. When a
 *	program sets up an exception vector, the BDOS squirrels it
 *	away in a data structure; the exception handler is expected
 *	to look up the vectors in that structure.
 *
 *	A user program can call this function via the BDOS function
 *	that calls a BIOS function.
 *
 * revisions:
 *
 *	2013-06-04 rli: original version.
 *
 * formal parameters:
 *
 *	- vector: The number of the vector that is to be set.
 *
 *	- Handler: A pointer to the function that is to be called
 *	  when this exception happens.
 *
 * return value:
 *
 *	- A pointer to the previous function that was called for this 
 *	  exception.
 *
 ***/

void *platform_setexc( unsigned short int vector, void *handler );

/***
 *
 * platform_ccpdecode
 *
 *	If config_hasccphooks_c is defined, the CCP calls this function
 *	when it sees a token that it does not recognize. This allows the
 *	platform-specific code to add commands to the CCP.
 *
 *	Command decoding has two parts:
 *
 *	- This function, which turns a token into a command number.
 *	- platform_ccpexecute, which executes the command by number.
 *
 * revisions:
 *
 *	2013-06-05 rli: original version.
 *
 * formal parameters:
 *
 *	- token: supplies a pointer to the token that has been found.
 *
 * return value:
 *
 *	- 0: the token is not recognized.
 *
 *	- else: the command number corresponding to this token.
 *
 ***/

unsigned short int platform_ccpdecode( signed char *token );

/***
 *
 * platform_ccpexecute
 *
 *	If config_hasccphooks_c is defined, the CCP calls this function
 *	to execute a platform-specific command. This allows the 
 *	platfrom-specific code to add commands to the CCP.
 *
 *	Command decoding has two parts:
 *
 *	- platform_ccpdecode, which turns a token into a command number.
 *	- This function, which executes the command by number.
 *
 * revisions:
 *
 *	2013-06-05 rli: original version.
 *
 *	2013-06-16 rli: commandno is a short, not an int.
 *
 * formal parameters:
 *
 *	- commandno: Supplies the number of the command that is to be
 *	  executed. This should be a number that was returned by
 *	  platform_ccpdecode.
 *
 * return value:
 *
 *	- 0: The command number was not recognized; the CCP displays
 *	  an error message.
 8
 *	- else: the command has been executed.
 *
 ***/

unsigned int platform_ccpexecute( unsigned short int commandno );

#endif /* ndef config_h_included */
