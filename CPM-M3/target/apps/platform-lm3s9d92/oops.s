/***
 *
 * 	generates an exception to trigger an exception handler for debugging.
 *
 * revisions:
 *
 *	2013-12-29 rli: original attempt.
 *
 ***/

	.text
	.thumb
	.syntax unified
	
	.global glue_entry
glue_entry:
	mov r0,#0
	mov r1,#1
	mov r2,#2
	mov r3,#3
	mov r4,#4
	mov r5,#5
	mov r6,#6
	mov r7,#7
	mov r8,#8
	mov r9,#9
	mov r10,#10
	mov r11,#11
	mov r12,#12
	svc #0
	bx lr

	.end
