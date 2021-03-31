/**************************************************************
 *
 * file: bootvectors.s
 *
 *	This file contains a minimal vector table for use in the 
 *	boot environment. The idea is that we can be certain that
 *	interrupts will be disabled, so we don't need to supply
 *	interrupt vectors.
 *
 * revisions:
 *
 *	2013-12-20 rli: original attempt.
 *
 **************************************************************/

	.text

/***
 *
 * bootvectors
 *
 *	A minimal vector table for use by the bootstrap.
 *
 * revisions:
 *
 *	2013-12-20 rli: original attempt.
 *
 ***/

	.extern bstrap
	.extern bstrap_exception

	.global bootvectors

bootvectors:
	.long 0x20001000	/*  0: stack pointer */
	.long bstrap		/*  1: reset */
	.long bstrap_exception	/*  2: non-maskable interrupt */
	.long bstrap_exception	/*  3: hard fault */
	.long bstrap_exception	/*  4: memory management */
	.long bstrap_exception	/*  5: bus fault */
	.long bstrap_exception	/*  6: usage fault */
	.long bstrap_exception	/*  7: reserved */
	.long bstrap_exception	/*  8: reserved */
	.long bstrap_exception	/*  9: reserved */
	.long bstrap_exception	/* 10: reserved */
	.long bstrap_exception	/* 11: SVC */
	.long bstrap_exception	/* 12: debug monitor */
	.long bstrap_exception	/* 13: reserved */
	.long bstrap_exception	/* 14: PendBV */
	.long bstrap_exception	/* 15: SysTick */

	.end
