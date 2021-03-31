/*************************************************************
 *
 * file: platform-lm3s9d92.ld
 *
 *	This file is the ld script for CP/M applications running on an
 *	LM3S9D92.
 *
 *	TPA starts at 0x20000100 (leaving space for the base page at
 *	the start of RAM, 0x20000000).
 *
 * revisions:
 *
 *	2014-09-13 rli: original attempt, based on the ld script for
 *	  the operating system.
 *
 **********************************************************/

SECTIONS
{
  .text 0x20000100 : ALIGN( 4 ) {
    *(.text)
    *(.rodata)
    *(.data)
  }
  .bss : ALIGN( 4 ) {
    *(.bss)
    *(COMMON)
  }

  __bss_start__ = ADDR(.bss);
  __bss_end__ = ADDR(.bss)+SIZEOF(.bss);
  __end__ = ADDR(.bss)+SIZEOF(.bss);
}
