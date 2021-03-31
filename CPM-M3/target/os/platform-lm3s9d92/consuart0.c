/*******************************************************************
 *
 * file: consuart0.c
 *
 *	This file contains a console device driver for UART 0 of an
 *	LM3S9D92.
 *
 * revisions:
 *
 *	2013-12-21 rli: original, based on consnull.
 *
 ******************************************************************/

/*****************************
 *
 *	INCLUDES
 *
 *****************************/

#include "config.h"
#include "bios.h"
#include "modubios.h"
#include "cpureg.h"

/*****************************
 *
 *	PROTOTYPES
 *
 *****************************/

void consuart0_cboot( void );
void consuart0_wboot( void );
void consuart0_close( void );
char consuart0_conin( void );
void consuart0_conout( char victim );
unsigned short int consuart0_conist( void );
unsigned short int consuart0_conost( void );

/*****************************
 *
 *	GLOBALS
 *
 *****************************/

/***
 *
 * consuart0
 *
 *	This structure describes the device to the modular bios. It
 *	supplies the BIOS with pointers to each of the services offered
 *	by the device.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 ***/

modubios_cons_t consuart0 = {
  0x30323964,		/* "d920" */
  0x20131221,
  consuart0_cboot,
  consuart0_wboot,
  consuart0_close,
  consuart0_conin,
  consuart0_conout,
  consuart0_conist,
  consuart0_conost,
  0
};

/*****************
 *
 *	CODE
 *
 *****************/

/***
 *
 * consuart0_cboot
 *
 *	This function is called during cold boot, before much
 *	initialization has been performed. It is intended to prepare
 *	the port for use.
 *
 *	Since the console is used to display the copyright message
 *	before the BDOS is initialized, this function cannot assume
 *	BDOS services are available. A warm boot is performed during
 *	the cold boot sequence after the BDOS has been initialized;
 *	actions that need the BDOS can be tucked there.
 *
 *	For the moment, we're running on the LM3S9D92's precision internal
 *	oscillator at 16MHz. We need to:
 *
 *	- Enable the clock to GPIOA, which controls the pins to which
 *	  our UART is attached.
 *
 *	- Configure the pins to which our UART is attached.
 *
 *	- Enable the clock to the UART.
 *
 *	- Set the UART's baud rate.
 *
 *	- Enable the transmitter and receiver.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version. no fifos for now.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	none.
 *
 ***/

void consuart0_cboot( void )
{
  volatile unsigned int temp;

  /* Enable the clock to GPIOA, then read the register back to ensure
   * a couple of cycles happen before we attempt to access the hardware.
   */

  cpureg_rcgc2_g |= cpureg_rcgc2_gpioa_m;
  temp = cpureg_rcgc2_g;

  /* We need to set the UART pins to digital and to be controlled by
   * their alternate function.
   *
   * The manual says:
   *
   * - direction bits are irrelevant.
   * - by default, all pins have 2mA drive, which should be sufficient.
   * - the default alternate function is UART0, so we don't need to
   *   set that register.
   *
   * so those should be sufficient.
   */

  cpureg_gpioa_afsel_g = ( 1 << 1 ) | ( 1 << 0 );
  cpureg_gpioa_den_g = ( 1 << 1 ) | ( 1 << 0 );

  /* Enable the clock to UART0, then read the register back to ensure
   * a couple of cycles happen before we attempt to access the hardware.
   */

  cpureg_rcgc1_g |= cpureg_rcgc1_uart0_m;
  temp = cpureg_rcgc1_g;

  /* Set the baud rate. 115,200 for now.
   */

  cpureg_uart0_ibrd_g = cpureg_uart_ibrd_115200_c;
  cpureg_uart0_fbrd_g = cpureg_uart_fbrd_115200_c;

  /* Set data size, stop bits, etc; accessing this register also commits
   * the change to the baud rate. Most of the defaults are fine; we just
   * need to set the data size to 8 bits (default is 5 bits, because
   * zeros are easy).
   */

  cpureg_uart0_lcrh_g = cpureg_uart_lcrh_wlen_8_c;

  /* Enable the UART, along with transmission and reception.
   */

  cpureg_uart0_ctl_g = cpureg_uart_ctl_rxe_m |
    cpureg_uart_ctl_txe_m | cpureg_uart_ctl_uarten_m;

  /* And that should suffice.
   */

  return;

}

/***
 *
 * consuart0_wboot
 *
 *	This function is called during a warm boot. It is intended to
 *	allow the device to handle things that need to be done at that
 *	time; for example, a list device using an automagic printer
 *	switch might flush its buffer and release ownership of the
 *	switch.
 *
 * 	A warm boot is performed during the cold boot sequence after the
 *	BDOS has been initialized.
 *
 * revisions:
 *
 *	2010-08-06 rli: original version.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	none.
 *
 ***/

void consuart0_wboot( void )
{
}

/***
 *
 * consuart0_conin
 *
 *	This function waits for a character to arrive from the device
 *	and returns that character.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- The received character.
 *
 ***/

char consuart0_conin( void )
{
  while( consuart0_conist() == 0 ) 
  {
  }
  return (char)( cpureg_uart0_dr_g );
}

/***
 *
 * consuart0_conout
 *
 * 	This function sends a character to the device. If the device
 *	is not ready to accept a character, this function waits for
 *	it to become ready.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 * parameters:
 *
 *	- victim: supplies the character that is to be sent.
 *
 * return value:
 *
 *	none.
 *
 ***/

void consuart0_conout( char victim )
{
  while( consuart0_conost() == 0 )
  {
  }
  cpureg_uart0_dr_g = ( (int)( victim ) ) & 0xff;
  return;
}

/***
 *
 * consuart0_conist
 *
 *	This function examines the device to see if a character has
 *	been received.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- 0: A character has not been received from the device.
 *
 *	- 255: A character is available from the device.
 *
 ***/

unsigned short int consuart0_conist( void )
{
  if( ( cpureg_uart0_fr_g & cpureg_uart_fr_rxfe_m ) == 0 ) {
    return 255;
  }
  return 0;
}

/***
 *
 * consuart0_conost
 *
 *	This function examines the device to see whether it is ready to
 *	accept a device.
 *
 * revisions:
 *
 *	2013-12-21 rli: original version.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	- 0: The device is not ready to accept a character.
 *
 *	- 255: The device can accept a character.
 *
 ***/

unsigned short int consuart0_conost( void )
{
  if( ( cpureg_uart0_fr_g & cpureg_uart_fr_txff_m ) == 0 ) {
    return 255;
  }
  return 0;
}

/***
 *
 * consuart0_close
 *
 *	This function is called when the IOBYTE is changed away from
 *	this device. It gives the driver an opportunity to do some
 *	finalization.
 *
 * revisions:
 *
 *	2010-08-08 rli: original version.
 *
 * parameters:
 *
 *	none.
 *
 * return value:
 *
 *	none.
 *
 ***/

void consuart0_close( void )
{
}

