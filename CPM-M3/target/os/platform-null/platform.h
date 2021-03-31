/********************************************************************
 *
 * file: platform.h
 *
 *	This file supplies platform-specific definitions relied upon
 *	by the rest of the system.
 *
 * revisions:
 *
 *	2013-05-30 rli: original version.
 *
 *	2013-06-01 rli: moved declaration of MRT, console, and disk
 *	  tables here.
 *
 *	2013-06-02 rli: add a quiesce routine called by warmboot that
 *	  allows us to ensure unused hardware that scribbles on 
 *	  memory has been turned off.
 *
 *	2013-06-04 rli: moved configuration constants and some
 *	  prototypes to config.h.
 *
 ********************************************************************/

#ifndef platform_h_Included
#define platform_h_Included

/****************
 *
 *	GLOBALS
 *
 ****************/

/***
 *
 * platform_mrt
 *
 *      Holds the memory region table for the system.
 *
 *	This is referred by modubios. Other bioses may make other
 *	arrangements to supply the memory region table.
 *
 * revisions:
 *
 *      2013-04-29 rli: decided to make external.
 *
 *	2013-06-01 rli: moved here and renamed.
 *
 ***/

extern bios_mrt_t platform_mrt;

/***
 *
 * platform_consoles
 *
 *      This table is used by modubios to find the console drivers.
 *
 * revisions:
 *
 *      2013-04-29 rli: decided to make external.
 *
 *	2013-06-01 rli: moved here and renamed.
 *
 ***/

extern modubios_cons_t *platform_consoles[];

/***
 *
 * platform_disks
 *
 *      This table is used by modubios to find the disk drivers.
 *
 * revisions:
 *
 *      2013-04-29 rli: decided to make external.
 *
 *	2013-06-01 rli: moved here and renamed.
 *
 ***/

extern modubios_disk_t *platform_disks[];

/***
 *
 * traphndl
 *
 *	If config_hastrphndl_c is defined, the glue code leaves this
 *	function undefined, allowing it to be overridden by the
 *	platform-specific code.
 *
 *	During cold boot, the BDOS sets this up as the trap 2 handler by
 *	calling bios_setexc( trap2v, traphndl ). The value of trap2v can
 *	be controlled using config_trap2v_c; it is, by default, 34.
 *
 *	The BDOS presumes that this function captures software
 *	interrupts and turns them into BDOS calls.
 *
 *	I don't know why it defines the function as returning something.
 *	Perhaps the C dialect in which the BDOS is written has no
 *	concept of void.
 *
 * revisions:
 *
 *	2013-06-05 rli: original version.
 *
 * formal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	beats me. This function is expected to be a trap handler, which
 *	means that is invoked by an exception and returns via the
 *	exception-handling mechanism. I.e., it doesn't actually return
 *	anything.
 *
 ***/

unsigned char *traphndl( void );

#endif /* ndef platform_h_Included */
