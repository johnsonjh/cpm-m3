/****************************************************************************************
 *
 * file: arch-host.c
 *
 *	This file contains a C version of the assembly language function that in the
 *	target port sets the stack pointer and calls a specified function. For the
 *	host, we don't set the stack pointer. However, that means we have a bit of
 *	a stack leak whenever we enter a function (i.e., at each cold or warm boot).
 *
 * revisions:
 *
 *	2013-06-16 rli: original version.
 *
 ***************************************************************************************/

/***********************
 *
 *	INCLUDES
 *
 ***********************/

#include "config.h"
#include "cpm.h"
#include "arch.h"

/***********************
 *
 *	CODE
 *
 ***********************/

/***
 *
 * arch_enter
 *
 *	This function is intended to set the stack pointer and enter a specified
 *	function. On the host, we just enter the function.
 *
 * revisions:
 *
 *	2013-06-16 rli: original version.
 *
 * formal parameters:
 *
 *	- BasePage: Passed to the called program.
 *
 *	- EntryPoint: A pointer to the function that is to be called.
 *
 *	- StackPointer: Not used.
 *
 * informal parameters:
 *
 *	none.
 *
 * return value:
 *
 *	This function does not return.
 *
 * notes:
 *
 *	- if the called function returns, warmboot is entered.
 *
 ***/

void arch_enter( cpm_basepage_t *BasePage,
  void (*EntryPoint)( cpm_basepage_t *BasePage ),
  void *StackPointer )
{
  EntryPoint( BasePage );
  warmboot();
}

