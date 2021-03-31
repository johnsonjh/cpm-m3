/************************************************************
 *
 * file: header.s
 *
 *	This is the CP/M-ARM executable header. It consists of four
 *	longwords containing:
 *
 *	- A branch to the C entry point, locore_entry. The primitive 
 *	  .COM-style program load enters the program here.
 *	- An architecture tag, marking the program as being an
 *	  ARM executable.
 *	- A region tag, indicating where the program wants to be
 *	  loaded in memory. This is intended to be used to prevent
 *	  an image from being loaded on a CP/M-ARM system with an
 *	  incompatible memory map.
 *	- The address of the entry point. This is primarily to ensure
 *	  that Thumb architectures can force bit 0 on; calculating
 *	  the entry point otherwise would require knowing whether
 *	  we're running on Thumb.
 *
 *	Since CP/M has set the stack up for us, there isn't really
 *	anything that *must* be done in assembly language.
 *
 * revisions:
 *
 *	2010-08-20 rli: original version.
 *
 *	2013-12-22 rli: entry pointer.
 *
 **************************************************************/

	.text
	.thumb
	
	.global _start
	.extern glue_entry

_start:
	b glue_entry
	.align 2
	.long 0x0000334d
	.long _start
	.long _start + 1

	.end
