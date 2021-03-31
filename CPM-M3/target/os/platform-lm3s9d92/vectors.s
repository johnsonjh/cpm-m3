/**************************************************************
 *
 * file: vectors.s
 *
 *	This file contains the vector table and low-level exception
 *	handler. The low-level handler builds a structure describing
 *	the processor state at the time of the exception and passes
 *	it along to the high-level handler.
 *
 * revisions:
 *
 *	2013-12-22 rli: original attempt.
 *
 **************************************************************/

	.text
	.syntax unified

/***
 *
 * vectors
 *
 *	A vector table for use when the system is up.
 *
 * revisions:
 *
 *	2013-12-22 rli: original attempt.
 *
 *	2013-12-23 rli: forgot to add one because Thumb.
 *
 ***/

	.global vectors

vectors:
	.long 0x20001000	/*  0: stack pointer */
	.long llexception+1	/*  1: reset */
	.long llexception+1	/*  2: non-maskable interrupt */
	.long llexception+1	/*  3: hard fault */
	.long llexception+1	/*  4: memory management */
	.long llexception+1	/*  5: bus fault */
	.long llexception+1	/*  6: usage fault */
	.long llexception+1	/*  7: reserved */
	.long llexception+1	/*  8: reserved */
	.long llexception+1	/*  9: reserved */
	.long llexception+1	/* 10: reserved */
	.long llexception+1	/* 11: SVC */
	.long llexception+1	/* 12: debug monitor */
	.long llexception+1	/* 13: reserved */
	.long llexception+1	/* 14: PendBV */
	.long llexception+1	/* 15: SysTick */

	.long llexception+1	/* 16: interrupt 0 */
	.long llexception+1	/* 17: interrupt 1 */
	.long llexception+1	/* 18: interrupt 2 */
	.long llexception+1	/* 19: interrupt 3 */
	.long llexception+1	/* 20: interrupt 4 */
	.long llexception+1	/* 21: interrupt 5 */
	.long llexception+1	/* 22: interrupt 6 */
	.long llexception+1	/* 23: interrupt 7 */

	.long llexception+1	/* 24: interrupt 8 */
	.long llexception+1	/* 25: interrupt 9 */
	.long llexception+1	/* 26: interrupt 10 */
	.long llexception+1	/* 27: interrupt 11 */
	.long llexception+1	/* 28: interrupt 12 */
	.long llexception+1	/* 29: interrupt 13 */
	.long llexception+1	/* 30: interrupt 14 */
	.long llexception+1	/* 31: interrupt 15 */

	.long llexception+1	/* 32: interrupt 16 */
	.long llexception+1	/* 33: interrupt 17 */
	.long llexception+1	/* 34: interrupt 18 */
	.long llexception+1	/* 35: interrupt 19 */
	.long llexception+1	/* 36: interrupt 20 */
	.long llexception+1	/* 37: interrupt 21 */
	.long llexception+1	/* 38: interrupt 22 */
	.long llexception+1	/* 39: interrupt 23 */

	.long llexception+1	/* 40: interrupt 24 */
	.long llexception+1	/* 41: interrupt 25 */
	.long llexception+1	/* 42: interrupt 26 */
	.long llexception+1	/* 43: interrupt 27 */
	.long llexception+1	/* 44: interrupt 28 */
	.long llexception+1	/* 45: interrupt 29 */
	.long llexception+1	/* 46: interrupt 30 */
	.long llexception+1	/* 47: interrupt 31 */

	.long llexception+1	/* 48: interrupt 32 */
	.long llexception+1	/* 49: interrupt 33 */
	.long llexception+1	/* 50: interrupt 34 */
	.long llexception+1	/* 51: interrupt 35 */
	.long llexception+1	/* 52: interrupt 36 */
	.long llexception+1	/* 53: interrupt 37 */
	.long llexception+1	/* 54: interrupt 38 */
	.long llexception+1	/* 55: interrupt 39 */

	.long llexception+1	/* 56: interrupt 40 */
	.long llexception+1	/* 57: interrupt 41 */
	.long llexception+1	/* 58: interrupt 42 */
	.long llexception+1	/* 59: interrupt 43 */
	.long llexception+1	/* 60: interrupt 44 */
	.long llexception+1	/* 61: interrupt 45 */
	.long llexception+1	/* 62: interrupt 46 */
	.long llexception+1	/* 63: interrupt 47 */

	.long llexception+1	/* 64: interrupt 48 */
	.long llexception+1	/* 65: interrupt 49 */
	.long llexception+1	/* 66: interrupt 50 */
	.long llexception+1	/* 67: interrupt 51 */
	.long llexception+1	/* 68: interrupt 52 */
	.long llexception+1	/* 69: interrupt 53 */
	.long llexception+1	/* 70: interrupt 54 */

/***
 *
 * llexception
 *
 *	This is the low-level exception handler. When the excpetion is
 *	taken, the processor stacks some of the registers (those that are
 *	modified to enter interrupt state or that must be saved before calling
 *	a function) and enters this function. 
 *
 *	This function saves the remainder of the general-purpose registers
 *	and calls the high-level exception handler, passing the base address
 *	of the structure to that handler.
 *
 *	The high-level exception handler will see its first parameter as
 *	a pointer to a structure with the following layout:
 *
 *
 * 	- xPSR
 * 	- pc
 * 	- lr
 * 	- r12
 * 	- r3
 * 	- r2
 * 	- r1
 * 	- r0  <-- hardware-saved context starts here
 * 	- r11
 * 	- r10
 * 	- r9
 * 	- r8
 * 	- r7
 * 	- r6
 * 	- r5
 * 	- r4
 *
 * revisions:
 *
 *	2013-12-22 rli: original attempt.
 *
 *	2013-12-29 rli: botched SP adjustment.
 *
 ***/


llexception:
	push {r4,r5,r6,r7,r8,r9,r10,r11}
	mov r0,sp
	ldr r1,llexception_trampoline
	bx r1

	.extern exception
llexception_trampoline:
	.long exception

	.end
