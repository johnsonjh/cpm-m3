#-----------------------------------------------------------------
#
# file: arch-arm-noicache.s
#
#	This file contains assembly-language stuff relied upon by
#	the architecture-independent glue. This variant is intended
#	for systems that don't have an instruction cache.
#
# revisions:
#
#	2010-08-01 rli: original version.
#
#	2010-08-02 rli: extend the header to describe all sections.
#
#	2010-08-03 rli: begin some boot sequence rework; boot_setsp
#	  will initialize the stack pointer, we'll enter the BIOS
#	  at bios_cboot.
#
#	2013-05-29 rli: renamed to arch-arm.s and all the stuff in
#	  here to begin with arch_. boot_entry will be moved to
#	  the platform-specific stuff.
#
#	2013-05-30 rli: liberated arch-arm-icache.s and hacked over.
#
#	2013-06-09 rli: liberated arch-arm-noicache.s. cs doesn't
#	  recognize b.w.
#
#-----------------------------------------------------------------

	.text
#---
#
# arch_enter
#
#	This function initializes the stack pointer and enters a
#	function at a specific address; it is used to enter a program
#	loaded into the TPA.
#
#	If the loaded program returns, the system is restarted.
#
# revisions:
#
#	2010-08-01 rli: original version.
#
#	2010-08-15 rli: caller needs to pass us the initial SP. Perform
#	  a warm boot if the program returns.
#
#	2010-08-21 rli: invalid icache before entering program.
#
#	2010-08-22 rli: use BDOS warmboot entry instead of BIOS. It
#	  takes a single parameter, which is set to 0 when a user
#	  calls the BDOS service 0.
#
#	2013-05-29 rli: renamed to arch_enter.
#
#	2013-05-30 rli: we have no icache to invalidate.
#
#	2013-06-09 rli: use a register to get to warmboot because
#	  thumb has short branches and codesourcery doesn't recognize
#	  b.w.
#
# inputs:
#
#	- r0: A parameter to be passed to the program. This is typically
#	  a pointer to the base page.
#
#	- r1: The address at which the program is to be entered.
#
#	- r2: The value that should be loaded into the stack pointer
#	  before the program is entered.
#
# outputs:
#
#	this function does not return.
#
#---

	.global arch_enter
	.extern warmboot

arch_enter:

	# set up sp and enter program

	mov sp,r2
	blx r1

	# program returned; do a warm boot

	eor r0,r0,r0
	ldr r1,warmboot_p
	bx r1

	.align 4
warmboot_p:
	.long warmboot

	.end
