/****************************************************************
 *
 * file: setjmp.s
 *
 *	This file contains implementations of setjmp and longjmp for
 *	ARM.
 *
 * revisions:
 *
 *	2010-08-20 rli: original version.
 *
 *	2013-12-22 rli: move SP into another register before saving it.
 *
 ***************************************************************/

	.text
	.syntax unified

/***
 *
 * setjmp
 *
 *	This function saves registers that are expected to be saved
 *	across a function call, plus the stack pointer, into a block of 
 *	memory. They can be restored later by longjmp.
 *
 *	An ARM function is allowed to munch r0 through r3 and r12. The
 *	PC is not saved; longjmp will return to the address in the
 *	link register. Consequently, 10 longwords are required to store
 *	the context.
 *
 * revisions:
 *
 *	2010-08-21 rli: original attempt. 
 *
 *	2013-12-22 rli: move SP into r1 before saving.
 *
 * inputs:
 *
 *	- r0: The address of the block into which the context is to be
 *	  stored. This is assumed to be appropriately aligned.
 *
 * outputs:
 *
 *	- r0: zero.
 *
 ***/

	.global setjmp

setjmp:
	mov r1,sp
	stmia r0!,{r1,r4-r11,r14}
	eor r0,r0,r0
	bx r14

/***
 *
 * longjmp
 *
 *	This function restores registers from a block of memory
 *	previously constructed by setjmp, then returns to the 
 *	address held in the restored link register. The effect is
 *	as if setjmp had returned when the longjmp is performed.
 *
 * revisions:
 *
 *	2010-08-21 rli: original attempt. 
 *
 *	2013-12-22 rli: restore SP from r2.
 *
 * inputs:
 *	
 *	- r0: The address of the block from which the registers are to
 *	  be loaded.
 *
 *	- r1: The value that should be returned. Since setjmp always
 *	  returns zero, this can be used by setjmp's caller to
 *	  distinguish between setjmp and longjmp.
 *
 * outputs:
 *
 *	- Registers r4 through r11, r13, and r14 are restored to the
 *	  value they had when setjmp was called.
 *
 *	- r0 receives a copy of r1.
 *
 ***/

	.global longjmp

longjmp:
	ldmia r0!,{r2,r4-r11,r14}
	mov sp,r2
	mov r0,r1
	bx r14

	.end

