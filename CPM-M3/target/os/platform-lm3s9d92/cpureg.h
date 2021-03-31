/*******************************************************************
 *
 * file: cpureg.h
 *
 *	This file contains declarations related to the hardware registers
 *	in the CPU.
 *
 * revisions:
 *
 *	2013-12-21 rli: original, only registers we're interested
 *	  in dealing with.
 *
 *	2013-12-28 rli: flash memory controller registers.
 *
 *******************************************************************/

#ifndef cpureg_h_included
#define cpureg_h_included

/********************
 *
 *	GLOBALS
 *
 ********************/

/***
 *
 * cpureg_*_g
 *
 *	These constants allow access to the registers.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version. peripherals accessed via APB.
 *
 *	2013-12-28 rli: flash memory controller. FWB is intended to be
 *	  used as a 32-longword array.
 *
 *	2014-01-01 rli: added cpureg_ram_g, which is an array over
 *	  the internal ram.
 *
 ***/

#define cpureg_rcgc1_g       ( *(volatile unsigned int *)0x400fe104 )
#define cpureg_rcgc2_g       ( *(volatile unsigned int *)0x400fe108 )

#define cpureg_uart0_dr_g    ( *(volatile unsigned int *)0x4000c000 )
#define cpureg_uart0_fr_g    ( *(volatile unsigned int *)0x4000c018 )
#define cpureg_uart0_ibrd_g  ( *(volatile unsigned int *)0x4000c024 )
#define cpureg_uart0_fbrd_g  ( *(volatile unsigned int *)0x4000c028 )
#define cpureg_uart0_lcrh_g  ( *(volatile unsigned int *)0x4000c02c )
#define cpureg_uart0_ctl_g   ( *(volatile unsigned int *)0x4000c030 )

#define cpureg_gpioa_afsel_g ( *(volatile unsigned int *)0x40004420 )
#define cpureg_gpioa_den_g   ( *(volatile unsigned int *)0x4000451c )

#define cpureg_cpuid_g       ( *(volatile unsigned int *)0xe000ed00 )
#define cpureg_intctrl_g     ( *(volatile unsigned int *)0xe000ed04 )
#define cpureg_vtor_g        ( *(volatile unsigned int *)0xe000ed08 )
#define cpureg_apint_g       ( *(volatile unsigned int *)0xe000ed0c )

#define cpureg_fma_g         ( *(volatile unsigned int *)0x400fd000 )
#define cpureg_fmd_g         ( *(volatile unsigned int *)0x400fd004 )
#define cpureg_fmc_g         ( *(volatile unsigned int *)0x400fd008 )
#define cpureg_fcris_g       ( *(volatile unsigned int *)0x400fd00c )
#define cpureg_fcim_g        ( *(volatile unsigned int *)0x400fd010 )
#define cpureg_fcmisc_g      ( *(volatile unsigned int *)0x400fd014 )
#define cpureg_fmc2_g        ( *(volatile unsigned int *)0x400fd020 )
#define cpureg_fwbval_g      ( *(volatile unsigned int *)0x400fd030 )
#define cpureg_fctl_g        ( *(volatile unsigned int *)0x400fd0f8 )
#define cpureg_fwb_g         (  (volatile unsigned int *)0x400fd100 )

#define cpureg_ram_g         (  (volatile unsigned char *)0x20000000 )

/********************
 *
 *	CONSTANTS
 *
 ********************/

/***
 *
 * cpureg_rcgc2_*
 *
 *	Symbols for interesting stuff in the RCGC2 registers.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 ***/

#define cpureg_rcgc2_gpioa_m ( 1 << 0 )

/***
 *
 * cpureg_rcgc1_*
 *
 *	Symbols for interesting stuff in the RCGC1 register.
 *
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 ***/

#define cpureg_rcgc1_uart0_m ( 1 << 0 )

/***
 *
 * cpureg_uart_ibrd_*_c
 * cpureg_uart_fbrd_*_c
 *
 *	These constants are baud rate divisors. The baud rate divider
 *	has two parts: an integr part and a fractional part.
 *
 *	Baud rate is calculated as:
 *
 *	BRD = BRDI + BRDF = UARTSysClk / ( ClkDiv * BaudRate )
 *
 *	where BRDI is the integer part and BRDF is the fractional part.
 *	UARTSysClk is the system clock rate; by default, we run from
 *	the precision internal oscillator at 16MHz. ClkDiv can be either
 *	8 or 16, depending on UART configuration; by default, it's 16.
 *	BaudRate is, of course, the baud rate.
 *
 *	The value to be written to the fbrd register is calculated as:
 *
 *	UARTFBRD = integer( BRDF * 64 + 0.5 )
 *
 * revisions:
 *
 *	2013-12-21 rli: original version for 16MHz; 115200 only for now.
 *
 ***/

#define cpureg_uart_ibrd_115200_c 8
#define cpureg_uart_fbrd_115200_c 44

/***
 *
 * cpureg_uart_lcrh_*
 *
 *	Symbols for stuff in a UART line control register.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 ***/

#define cpureg_uart_lcrh_brk_m    ( 1 << 0 )
#define cpureg_uart_lcrh_pen_m    ( 1 << 1 )
#define cpureg_uart_lcrh_eps_m    ( 1 << 2 )
#define cpureg_uart_lcrh_stp2_m   ( 1 << 3 )
#define cpureg_uart_lcrh_fen_m    ( 1 << 4 )
#define cpureg_uart_lcrh_wlen_s   2
#define cpureg_uart_lcrh_wlen_v   5
#define cpureg_uart_lcrh_wlen_m   ( 3 << cpureg_uart_lcrh_wlen_v )
#define cpureg_uart_lcrh_wlen_8_c ( 3 << cpureg_uart_lcrh_wlen_v )
#define cpureg_uart_lcrh_wlen_7_c ( 2 << cpureg_uart_lcrh_wlen_v )
#define cpureg_uart_lcrh_wlen_6_c ( 1 << cpureg_uart_lcrh_wlen_v )
#define cpureg_uart_lcrh_wlen_5_c ( 0 << cpureg_uart_lcrh_wlen_v )
#define cpureg_uart_lcrh_sps_m    ( 1 << 7 )

/***
 *
 * cpureg_uart_ctl_*
 *
 *	Symbols for interesting stuff in a UART control register.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 ***/

#define cpureg_uart_ctl_uarten_m ( 1 << 0 )
#define cpureg_uart_ctl_hse_m    ( 1 << 5 )
#define cpureg_uart_ctl_txe_m    ( 1 << 8 )
#define cpureg_uart_ctl_rxe_m    ( 1 << 9 )

/***
 *
 * cpureg_uart_fr_*
 *
 *	Symbols for interesting stuff in a UART flag register.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 ***/

#define cpureg_uart_fr_busy_m ( 1 << 3 )
#define cpureg_uart_fr_rxfe_m ( 1 << 4 )
#define cpureg_uart_fr_txff_m ( 1 << 5 )
#define cpureg_uart_fr_rxff_m ( 1 << 6 )
#define cpureg_uart_fr_txfe_m ( 1 << 7 )

/***
 *
 * cpureg_intctrl_*
 *
 *	symbols for interesting stuff in the INTCTRL register.
 *
 * revisions:
 *
 *	2013-12-22 rli: original version.
 *
 ***/

#define cpureg_intctrl_vecact_v 0
#define cpureg_intctrl_vecact_s 7
#define cpureg_intctrl_vecact_m 0x0000007f

/***
 *
 * cpureg_apint_*
 *
 *	symbols for interesting stuff in the APINT register.
 *
 * revisions:
 *
 *	2013-12-22 rli: original version.
 *
 ***/

#define cpureg_apint_sysresreq_m ( 1 << 2 )
#define cpureg_apint_vectkey_v   16
#define cpureg_apint_vectkey_s   16
#define cpureg_apint_vectkey_m   0xffff0000
#define cpureg_apint_vectkey_c   0x05fa0000
#define cpureg_apint_sysresreq_c 0x05fa0004

/***
 *
 * cpureg_fmc_*
 *
 *	symbols for interesting stuff in the FMC register.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 ***/

#define cpureg_fmc_wrkey_v  16
#define cpureg_fmc_wrkey_s  16
#define cpureg_fmc_wrkey_m  0xffff0000
#define cpureg_fmc_wrkey_c  0xa4420000

#define cpureg_fmc_write_m  ( 1 << 0 )
#define cpureg_fmc_erase_m  ( 1 << 1 )
#define cpureg_fmc_merase_m ( 1 << 2 )
#define cpureg_fmc_comt_m   ( 1 << 3 )

#define cpureg_fmc_write_c ( cpureg_fmc_wrkey_c | cpureg_fmc_write_m )
#define cpureg_fmc_erase_c ( cpureg_fmc_wrkey_c | cpureg_fmc_erase_m )

/***
 *
 * cpureg_fmc2_*
 *
 *	symbols for interesting stuff in the FMC2 register.
 *
 * revisions:
 *
 *	2013-12-28 rli: original version.
 *
 ***/

#define cpureg_fmc2_wrkey_v 16
#define cpureg_fmc2_wrkey_s 16
#define cpureg_fmc2_wrkey_m 0xffff0000
#define cpureg_fmc2_wrkey_c 0xa4420000

#define cpureg_fmc2_wrbuf_m ( 1 << 0 )

#define cpureg_fmc2_wrbuf_c ( cpureg_fmc2_wrkey_c | cpureg_fmc2_wrbuf_m )

#endif /* ndef cpureg_h_included */

