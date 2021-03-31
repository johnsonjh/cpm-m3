/*****************************************************************
 *
 * file: exception.h
 *
 *	This file contains declarations related to exception handling.
 *
 * revisions:
 *
 *	2013-12-22 rli: original version.
 *
 *****************************************************************/

#ifndef exception_h_included
#define exception_h_included

/*************************
 *
 *	TYPES
 *
 *************************/

/***
 *
 * exception_t
 *
 *	A structure of this type is created when an exception occurs.
 *	Part of it is filled in by the hardware, the remainder by the
 *	low-level exception handler. A pointer to the structure is
 *	passed to the high-level exception handler.
 *
 * revisions:
 *
 *	2013-12-22 rli: first attempt.
 *
 ***/

typedef struct exception_s {

  /* this portion created by hardware. */

  unsigned int psr;
  unsigned int pc;
  unsigned int lr;
  unsigned int r12;
  unsigned int r3;
  unsigned int r2;
  unsigned int r1;
  unsigned int r0;

  /* this portion created by the low-level exception handler */

  unsigned int r4;
  unsigned int r5;
  unsigned int r6;
  unsigned int r7;
  unsigned int r8;
  unsigned int r9;
  unsigned int r10;
  unsigned int r11;

} exception_t;

#endif /* ndef exception_h_included */

