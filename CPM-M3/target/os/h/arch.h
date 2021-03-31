
/********************************************************************
 *
 * file: arch.h
 *
 *	This file provides declarations for the stuff provided by the
 *	architecture-specific assembly-language portion of the system.
 *
 * revisions:
 *
 *	2010-08-02 rli: includes and switch to genericized structure
 *	  names.
 *
 *	2010-08-03 rli: boot_setsp.
 *
 *	2010-08-16 rli: renaming.
 *
 *	2013-04-29 rli: update prototype for boot_enter because KEIL
 *	  is picky about pointers to functions.
 *
 *	2013-05-29 rli: renamed to arch.h and all the stuff here to
 *	  start with arch_. Removed boot_entry; that will be in the
 *	  platform-specific stuff.
 *
 *	2013-05-30 rli: removed arch_setsp; it's not used anymore.
 *
 ********************************************************************/

#ifndef arch_h_Included
#define arch_h_Included

/*****************
 *
 *	PROTOTYPES
 *
 *****************/

void arch_enter( cpm_basepage_t *BasePage, 
  void (*EntryPoint)( cpm_basepage_t *BasePage ),
  void *StackPointer );

#endif /* ndef arch_h_Included */
