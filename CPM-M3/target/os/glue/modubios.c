/**************************************************************
 *
 * file: modubios.c
 *
 *	This file is the core of a modular CP/M bios. It provides the
 *	interface between the operating system and two types of device
 *	drivers:
 *
 *	- console drivers, which drive a terminal, reader, punch, or
 *	  printer device.
 *
 *	- disk drivers.
 *
 * revisions:
 *
 *	2010-08-08 rli: start messing around.
 *
 *	2010-08-22 rli: call BDOS warm boot entry point when performing
 *	  a warm boot (i.e., at the end of cold boot and if the CCP
 *	  returns).
 *
 *	2010-08-22 rli: A BIOS descriptor.
 *
 *	2013-04-29 rli: start with only the null devices. assume clearing
 *	  of the .bss section is done by the loader (in this case, locore.c).
 *	  Move initializers for MRT and device arrays elsewhere; we'll
 *	  refer to them as externs in modubios.h and system-specific sources
 *	  will be responsible for providing them.
 *
 *	2013-05-01 rli: make the banner appendix more generic.
 *
 *	2013-05-02 rli: need to call cboot and wboot via boot_enter, because
 *	  they want to save clobbered registers on the stack.
 *
 *	2013-05-03 rli: use bdos function number constants from cpm.h.
 *
 *	2013-05-30 rli: boot_enter renamed to arch_enter, now declared
 *	  in arch.h. bios_getbios now returns a pointer to a generic
 *	  identification structure, which just magically happens to be
 *	  the same as the first two longwords of the modubios
 *	  descriptor.
 *
 *	2013-06-01 rli: Rename memory region table, console driver
 *	  table, and disk driver table platform_.
 *
 *	2013-06-02 rli: warmboot needs to ask the BDOS for the current
 *	  TPA limits, because a program can change them and these 
 *	  changes don't get reported to the BIOS. have warmboot call
 *	  a platform-specific quiesce routine.
 *
 *	2013-06-04 rli: moved configuration constants to config.h. Add a
 *	  callout for platform-specific exception setting.
 *
 *	2013-06-07 rli: bios_outzstring renamed glue_outzstring.
 *
 *************************************************************/

/******************
 *
 *	INCLUDES
 *
 ******************/

#include "config.h"
#include "bios.h"
#include "cpm.h"
#include "modubios.h"
#include "platform.h"
#include "arch.h"

/*******************
 *
 *	PROTOTYPES
 *
 *******************/

void warmboot( unsigned short int parm ); /* in bdosmisc.c */

/*******************
 *
 *	GLOBALS
 *
 *******************/

/***
 *
 * bios_iobyte
 *
 *	This variable contains the I/O byte, which is not currently
 *	implemented. Sure, we'll store it here and we'll read it back,
 *	but we aint using it to select any ports.
 *
 * revisions
 *
 *	2005-03-07 rli: added these comments.
 *
 *	2010-08-08 rli: initialization.
 *
 ***/

unsigned short int bios_iobyte =
  cpm_iobyte_con_tty_c |
  cpm_iobyte_rdr_tty_c |
  cpm_iobyte_pun_tty_c |
  cpm_iobyte_lst_tty_c;

/***
 *
 * bios_dirbuf
 *
 *	This is a scratch buffer used to hold sectors while the BDOS
 *	performs directory manipulation. It is shared between all
 *	of the disks.
 *
 * revisions:
 *
 *	2005-03-07 rli: Added these comments.
 *
 ***/
 
unsigned char bios_dirbuf[ 128 ];

/***
 *
 * bios_xfer
 *
 *	This structure describes a disk transfer. BIOS calls to set
 *	track, sector, and DMA address update this structure. A read or
 *	write will hand the structure off to the disk driver.
 *
 * revisions:
 *
 *	2010-08-12 rli: original version.
 *
 ***/

modubios_diskxfer_t bios_xfer;

/***
 *
 * bios_bios
 *
 *	This structure describes the BIOS. A pointer to this structure
 *	can be fetched via a user-callable service. The user can use
 *	this structure to identify the BIOS and find the drivers.
 *
 * revisions:
 *
 *	2010-08-22 rli: original version.
 *
 *	2013-05-01 rli: bump version.
 *
 *	2013-06-01 rli: console table renamed to platform_consoles,
 *	  disk table renamed to platform_disks.
 *
 *	2013-06-04 rli: platform tag constant moved to config.h.
 *
 ***/

modubios_bios_t bios_bios = {
  modubios_tag_c,
  modubios_version_c,
  config_archtag_c,
  platform_consoles,
  platform_disks
};


/***
 *
 * bios_cboot_entered
 *
 * 	FUNCTION 0: INITIALIZATION
 *
 *	This function is entered on cold boot and must initialize the
 *	BIOS. It initializes the stack pointer, initializes the BDOS,
 *	sets the default drive and user number, then enters the CCP.
 *
 *	NOTE:	This function is called via via boot_enter, which moves
 *		the stack pointer and calls the function.
 *		
 *
 * revisions:
 *
 *	2005-03-07 rli: Updated tese comments.
 *
 *	2010-08-03 rli: this becomes the entry point for the system.
 *
 *	2010-08-08 rli: inform console devices that we're booting before
 *	  printing the banner.
 *
 *	2010-08-12 rli: inform disks that we're booting.
 *
 *	2010-08-22 rli: pass drive number to disk driver boot routines.
 *
 *	2013-04-29 rli: assume .bss has already been cleared.
 *
 *	2013-05-01 rli: make the banner more generic.
 *
 *	2013-05-02 rli: make callable via boot_enter. stack pointer will
 *	  be set up by boot_enter, so we don't have to do it here.
 *
 *	2013-05-03 rli: use bdos function number constants from cpm.h.
 *	  Hmm, the numbers were doing it wrong; used drive no to set
 *	  user no and vice-versa.
 *
 *	2013-06-01 rli: refer to the console and disk tables via
 *	  bios_bios rather than by name.
 *
 *	2013-06-02 rli: allow the platform-specific code to define an
 *	  additional banner.
 *
 *	2013-06-04 rli: moved banner to config.h.
 *
 *	2013-06-07 rli: bios_outzstring renamed glue_outzstring.
 *
 * formal parameters:
 *
 *	- Basepage: Not used. This parameter exists solely to allow a
 *	  pointer to this function to be passed to boot_enter.
 *
 * informal parameters:
 *
 *	- bios_bootdrive_c: The number of the drive that is to become
 *	  the default.
 *
 *	- bios_bootuser_c: The user number that should be active when
 *	  the system starts.
 *
 *	- copyrt: The Digital Research copyright message.
 *
 *	- bios_bios: The bios descriptor. Among other things, contains
 *	  pointers to the disk and console tables.
 *
 *	- platform_banner_c: If this constant is defined, it supplies
 *	  a string that is displayed after the Digital Research CP/M-68K
 *	  copyright notice.
 *
 * return value:
 *
 *	This function does not return.
 *
 * side effects:
 *
 *	none.
 *
 ***/

void bios_cboot_entered( cpm_basepage_t *Basepage )
{

  int i;

  /* Give the consoles a chance to initialize themselves.
   */

  for( i = 0; i < 16; i++ ) {
    bios_bios.consoles[ i ]->cboot( );
  }

  /* Give the disks a chance to initialize themselves. This could
   * be, but is not, interleaved with the console drivers because
   * the disk drivers might want to display messages.
   */

  for( i = 0; i < 16; i++ ) {
    bios_bios.disks[ i ]->cboot( i );
  }

  /* Display the banner.
   */

  glue_outzstring( "\r\n" );
  glue_outzstring( copyrt );
  glue_outzstring( "\r\n" );

  /* If the platform has supplied an additional banner, display it.
   */

#ifdef config_banner_c
  glue_outzstring( config_banner_c );
  glue_outzstring( "\r\n" );
#endif

  /* Initialize the BDOS.
   */

  bdosinit();

  /* Set the default drive and user.
   */

  bdos( bdos_getsetusernumber_c, bios_bootuser_c );
  bdos( bdos_selectdisk_c, bios_bootdrive_c );

  /* do a warm boot. that'll enter the CCP.
   */

  warmboot( 0 );

}

/***
 *
 * bios_cboot
 *
 *	This is the cold boot entry point. It calls bios_cboot_entered
 *	via boot_enter, which sets the stack pointer.
 *
 * revisions:
 *
 *	2013-05-01 rli: original version.
 *
 *	2013-05-30 rli: boot_enter renamed to arch_enter.
 *
 *	2013-06-01 rli: bios_mrt renamed platform_mrt.
 *
 * formal parameters:
 *
 *	none.
 *
 * informal parameters:
 *
 *	- platform_mrt: The memory region table. It describes the start
 *	  address and size of the memory regions. The stack pointer is
 *	  initialized the address following the first memory region
 *	  (the stack pointer is decremented before data is pushed).
 *
 * return value:
 *
 *	this function does not return.
 *
 ***/

void bios_cboot( void )
{
  arch_enter( (void *)0, bios_cboot_entered, 
    (char *)platform_mrt.regions[ 0 ].base + 
    platform_mrt.regions[ 0 ].length );
}

/***
 *
 * bios_wboot_entered
 *
 *	FUNCTION 1: WARM BOOT
 *
 *	This function is called whenever a program terminates. 
 *
 *	Since the the system is up and running, we don't have to do
 *	much. We don't know where the stack is, so we have to
 *	re-initialize it and enter the ccp.
 *
 *	NOTE:	This function is called via boot_enter, which sets the
 *		stack pointer.
 *
 * revisions:
 *
 *	2010-08-01 rli: muck with comments to point out that entry was
 *	  moved. Also renamed that function to arch_entry.
 *
 *	2010-08-03 rli: rework of system startup.
 *
 *	2010-08-08 rli: inform the consoles that we're doing a warm
 *	  boot.
 *
 *	2010-08-12 rli: inform the disks that we're doing a warm boot.
 *
 *	2010-08-16 rli: use mrt to initialize stack pointer.
 *
 *	2010-08-22 rli: pass drive number to disk driver boot routines.
 *
 *	2013-05-02 rli: make callable via boot_enter.
 *
 *	2013-06-01 rli: find the console and disk tables via bios_bios
 *	  rather than directly.
 *
 *	2013-06-02 rli: call a platform-specific quiesce routine to
 *	  give the support code a chance to turn off hardware not used
 *	  by CP/M that may have been started by a user program (stuff
 *	  like an ethernet controller that scribbles on memory, for
 *	  example).
 *
 *	2013-06-04 rli: added a configuration constant that indicates
 *	  whether the platform has a quiesce function.
 *
 * formal parameters:
 *
 *	- Basepage: not used. this parameter is declared simply to allow
 *	  a pointer to this function to be passed to boot_enter.
 *
 * informal parameters:
 *
 *	- bios_bios: The bios descriptor. Contains pointers to the
 * 	  console and disk drivers.
 *
 * return value:
 *
 *	doesn't return.
 *
 ***/

void bios_wboot_entered( cpm_basepage_t *Basepage )
{
  int i;

  /* quiesce the platform.
   */

#ifdef config_hasquiesce_c
  platform_quiesce();
#endif

  /* let the consoles know we're doing a warm boot.
   */

  for( i = 0; i < 16; i++ ) {
    bios_bios.consoles[ i ]->wboot( );
  }

  /* let the disks know we're doing a warm boot.
   */

  for( i = 0; i < 16; i++ ) {
    bios_bios.disks[ i ]->wboot( i );
  }

  /* enter the CCP. if it returns, start over.
   */

  ccp();
  warmboot( 0 );

}

/***
 *
 * bios_wboot
 *
 *	This is the warm boot entry point. It calls bios_wboot_entered
 *	via boot_enter, which sets the stack pointer.
 *
 * revisions:
 *
 *	2013-05-01 rli: original version.
 *
 *	2013-05-30 rli: boot_enter renamed to arch_enter.
 *
 *	2013-06-01 rli: bios_mrt renamed platform_mrt.
 *
 *	2013-06-02 rli: we want the stack at the top of the TPA, but
 *	  the TPA limits can change; we need to ask the BIOS for the
 *	  current TPA limits.
 *
 * formal parameters:
 *
 *	none.
 *
 * informal parameters:
 *
 *	- platform_mrt: The memory region table. It describes the start
 *	  address and size of the memory regions. The stack pointer is
 *	  initialized the address following the first memory region
 *	  (the stack pointer is decremented before data is pushed).
 *
 * return value:
 *
 *	this function does not return.
 *
 ***/

void bios_wboot( void )
{
  static cpm_tpa_t tpab;

  /* ask the BDOS for the current TPA limits.
   */

  tpab.flags = 0;
  bdos( bdos_getsettpalimits_c, (unsigned int)&tpab );

  /* enter warm boot with the stack at the top of the TPA.
   */

  arch_enter( (void *)0, bios_wboot_entered, (char *)( tpab.top ) );
}

/***
 *
 * bios_const
 *
 * 	FUNCTION 2: CONSOLE STATUS
 *
 *	This function returns the status of the currently assigned
 *	console device. It returns 0x00FF when a character is ready
 *	to be read or 0x0000 when no console characters are ready.
 *
 * revisions:
 *
 *	2010-08-08 rli: redesign for modular consoles.
 *
 *	2013-06-01 rli: find the consoles via bios_bios.
 *
 * parameters:
 *
 *	none.
 *
 * globals:
 *
 *	- bios_iobyte: The IOBYTE.
 *
 *	- cpm_iobyte_con_m: This bitmask extracts the console field
 *	  of the IOBYTE.
 *
 *	- bios_bios: the bios descriptor. holds a pointer to the console
 *	  table.
 *
 * return value:
 *
 *	- 0x0000: no characters are ready from the console.
 *
 *	- 0x00ff: A character may be read from the console.
 *
 * side effects:
 *
 *	none.
 *
 ***/

unsigned short int bios_const( void )
{
  return bios_bios.consoles[ bios_iobyte & cpm_iobyte_con_m ]->conist();
}

/***
 *
 * bios_conin
 *
 * 	FUNCTION 3: READ CONSOLE CHARACTER
 *
 *	This function reads and returns the next console character.
 *	If no console character is ready, it waits until a character
 *	is typed before returning.
 *
 * revisions:
 *
 *	2010-08-08 rli: redesign for modular consoles.
 *
 *	2013-06-01 rli: find consoles via bios_bios.
 *
 * parameters:
 *
 *	none.
 *
 * globals:
 *
 *	- bios_iobyte: The IOBYTE.
 *
 *	- cpm_iobyte_con_m: This bitmask extracts the console field
 *	  of the IOBYTE.
 *
 *	- bios_bios: the bios descriptor. contains a pointer to the
 *	  console table.
 *
 * return value:
 *
 *	- character read from the console.
 *
 ***/

unsigned char bios_conin( void )
{
  return bios_bios.consoles[ bios_iobyte & cpm_iobyte_con_m ]->conin();
}

/***
 *
 * bios_conout
 *
 * 	FUNCTION 4: WRITE CONSOLE CHARACTER
 *
 *	This function sends a character to the console output device. If
 *	the console is not ready to send a character, this function
 *	waits until it is.
 *
 * revisions:
 *
 *	2010-08-08 rli: redesign for modular consoles.
 *
 *	2013-06-01 rli: find the consoles via bios_bios.
 *
 * parameters:
 *
 *	- victim: the character to be displayed.
 *
 * globals:
 *
 *	- bios_iobyte: The IOBYTE.
 *
 *	- cpm_iobyte_con_m: This bitmask extracts the console field
 *	  of the IOBYTE.
 *
 *	- bios_bios: the bios descriptor. contains a pointer to the
 *	  console table.
 *
 * return value:
 *
 *	none.
 *
 ***/

void bios_conout( unsigned char victim )
{
  bios_bios.consoles[ bios_iobyte & cpm_iobyte_con_m ]->conout( victim );
}

/***
 *
 * bios_list
 *
 * 	FUNCTION 5: LIST CHARACTER OUTPUT
 *
 *	This character sends an ASCII character to the currently
 *	designated list device.
 *
 * revisions:
 *
 *	2010-08-08 rli: redesign for modular consoles.
 *
 *	2013-06-01 rli: find the consoles via bios_bios.
 *
 * parameters:
 *
 *	- victim: the character to be sent to the printer.
 *
 * globals:
 *
 *	- bios_iobyte: the IOBYTE.
 *
 *	- cpm_iobyte_lst_m: This mask extracts the list field of the
 *	  iobyte.
 *
 *	- cpm_iobyte_lst_v: The number of the first bit in the list
 *	  field.
 *
 *	- bios_bios: the bios descriptor. contains a pointer to the
 *	  console table. the printers are consoles [12] through [15].
 *
 * return value:
 *
 *	none.
 *
 ***/

void bios_list( unsigned char victim )
{
  bios_bios.consoles[ ( ( bios_iobyte & cpm_iobyte_lst_m ) >> 
    cpm_iobyte_lst_v ) + 12 ]->conout( victim );
}

/***
 *
 * bios_punch
 *
 * 	FUNCTION 6: AUXILIARY OUTPUT
 *
 *	This function sends an ASCII character to the currently assigned
 *	auxiliary output device.
 *
 * revisions:
 *
 *	2010-08-08 rli: redesign for modular bios.
 *
 *	2013-06-01 rli: find the consoles via bios_bios.
 *
 * parameters:
 *
 *	- victim: the character to be sent to the punch.
 *
 * globals:
 *
 *	- bios_iobyte: the IOBYTE.
 *
 *	- cpm_iobyte_pun_m: This mask extracts the punch field of the
 *	  iobyte.
 *
 *	- cpm_iobyte_pun_v: The number of the first bit in the punch
 *	  field.
 *
 *	- bios_bios: the bios descriptor. contains a pointer to the
 *	  console table. the punches are consoles [8] through [11].
 *
 * return value:
 *
 *	none.
 *
 ***/

void bios_punch( unsigned char victim )
{
  bios_bios.consoles[ ( ( bios_iobyte & cpm_iobyte_pun_m ) >> 
    cpm_iobyte_pun_v ) + 8 ]->conout( victim );
}

/***
 *
 * bios_reader
 *
 * 	FUNCTION 7: AUXILIARY INPUT
 *
 *	This function reads the next character from the currently
 *	assigned auxiliary input device. It reports an end-of-file
 *	condition by returning an ASCII CTRL-Z (0x1a).
 *
 * revisions:
 *
 *	2010-08-08 rli: rework for modular consoles.
 *
 *	2013-06-01 rli: find the consoles via bios_bios.
 *
 * parameters:
 *
 *	none.
 *
 * globals:
 *
 *	- bios_iobyte: the IOBYTE.
 *
 *	- cpm_iobyte_rdr_m: This mask extracts the reader field of the
 *	  iobyte.
 *
 *	- cpm_iobyte_rdr_v: The number of the first bit in the reader
 *	  field.
 *
 *	- bios_bios: the bios descriptor. holds a pointer to the console
 *	  table. the readers are consoles [4] through [7].
 *
 * return value:
 *
 *	- The character read from the reader.
 *
 ***/

unsigned char bios_reader( void )
{
  return bios_bios.consoles[ ( ( bios_iobyte & cpm_iobyte_rdr_m ) >>
    cpm_iobyte_rdr_v ) + 4 ]->conin();
}

/***
 *
 * bios_listst
 *
 * 	FUNCTION 15: RETURN LIST STATUS
 *
 *	This function returns the status of the list device, either 0
 *	when the list device is not ready to accept a character or 0xff
 *	when a character can be sent to the list device.
 *
 * revisions:
 *
 *	2010-08-08 rli: rework for modular bios.
 *
 *	2013-06-01 rli: find the consoles via bios_bios.
 *
 * parameters:
 *
 *	none
 *
 * globals:
 *
 *	- bios_iobyte: the IOBYTE.
 *
 *	- cpm_iobyte_lst_m: This mask extracts the list field of the
 *	  iobyte.
 *
 *	- cpm_iobyte_lst_v: The number of the first bit in the list
 *	  field.
 *
 *	- bios_bios: the bios descriptor. holds a pointer to the console
 *	  table. the printers are consoles [12] through [15].
 *
 * return value:
 *
 *	- 0: printer is not ready to accept a character.
 *
 *	- 0xff: printer is ready to accept a character.
 *
 ***/

unsigned short int bios_listst( void )
{
  return bios_bios.consoles[ ( ( bios_iobyte & cpm_iobyte_lst_m ) >>
    cpm_iobyte_lst_v ) + 12 ]->conost();
}

/***
 *
 * bios_home
 *
 * 	FUNCTION 8: HOME
 *
 *	This function returns the disk head of the currently selected
 *	disk to the track 00 position.
 *
 * revisions:
 *
 *	2010-08-01 rli: added this date tag.
 *
 *	2010-08-12 rli: Update the description of the transfer. 
 *
 *	  NOTE: It looks to me like the BDOS no longer uses
 *	 	this service.
 *
 * formal parameters:
 *
 *	none.
 *
 * informal parameters:
 *
 *	- bios_xfer: Describes a disk transfer.
 *
 * return value:
 *
 *	none.
 *
 ***/

void bios_home( void )
{
  bios_xfer.track = 0;
}

/***
 *
 * bios_seldsk
 *
 * 	FUNCTION 9: SELECT DISK DRIVE
 *
 *	This function selects the specified disk for futher operations.
 *	In our case, we note the selected drive, then pass the operation
 *	on to the disk driver for further processing.
 *
 * revisions:
 *
 *	2010-08-01 rli: mucked about with these comments.
 * 
 *	2010-08-12 rli: use disk drivers.
 *
 *	2013-06-01 rli: find the disks via bios_bios.
 *
 * formal parameters:
 *
 *	- drive: the number of the drive to be selected. the range limit
 *	  is silently enforced.
 *
 *	- logged: indicates whether the drive is currently logged in.
 *
 * informal parameters:
 *
 *	- bios_xfer: Describes a disk transfer.
 *
 *	- bios_bios: the bios descriptor. holds a pointer to the disk table.
 *
 * return value:
 *
 *	- 0: The drive number is invalid.
 *
 *	- else: A pointer to the DPH for the drive.
 *
 ***/

bios_dph_t *bios_seldsk( unsigned char drive, unsigned char logged )
{
  drive &= 0xf;
  bios_xfer.drive = drive;
  return bios_bios.disks[ drive ]->select( drive, logged );
}

/***
 *
 * bios_settrk
 *
 * 	FUNCTION 10: SET TRACK NUMBER
 *
 *	This function specifies the disk track number for use in
 *	subsequent disk accesses. The track number is squirrelled away
 *	for future use.
 *
 * revisions:
 *
 *	2010-08-01 rli: dorked with these comments.
 *
 *	2010-08-12 rli: use disk drivers.
 *
 * formal parameters:
 *
 *	- track: The track number.
 *
 * informal parameters:
 *
 *	- bios_xfer: Describes a disk transfer.
 *
 * return value:
 *
 *	none.
 *
 ***/

void bios_settrk( unsigned short int track )
{
  bios_xfer.track = track;
}

/***
 *
 * bios_setsec
 *
 * 	FUNCTION 11: SET SECTOR NUMBER
 *
 *	This function specifies the sector number for subsequent disk
 *	accesses. The sector number is squirrelled away for future use.
 *
 * revisions:
 *
 *	2010-08-01 rli: dorked with this comment block.
 *
 *	2010-08-12 rli: disk drivers.
 *
 * formal parmaeters:
 *
 *	- sector: the sector to be used for subsequent I/O.
 *
 * informal parameters:
 *
 *	- bios_xfer: Describes a disk transfer.
 *
 * return value:
 *
 *	none.
 *
 ***/

void bios_setsec( unsigned short int sector )
{
  bios_xfer.sector = sector;
}

/***
 *
 * bios_setdma
 *
 * 	FUNCTION 12: SET DMA ADDRESS
 *
 *	This function specifies the data address for subsequent read or
 *	write operations. The address is squirrelled away for future
 *	use.
 *
 * revisions:
 *
 *	2010-08-01 rli: dorked with this comment block.
 *
 *	2010-08-12 rli: disk drivers.
 *
 * formal parameters:
 *
 *	- dmaaddress: pointer to the sector buffer.
 *
 * informal parameters:
 *
 *	- bios_xfer: describes a disk transfer.
 *
 * return value:
 *
 *	none.
 *
 ***/

void bios_setdma( void *dmaaddress )
{
  bios_xfer.buffer = dmaaddress;
}

/***
 *
 * bios_read
 *
 * 	FUNCTION 13: READ SECTOR
 *
 *	After the drive has been selected, the track has been set, the
 *	sector has been set, and the DMA address has been specified, the
 *	read function uses these parameters to read one sector and
 *	returns the error code.
 *
 *	The transfer is passed off to the disk driver for further
 *	processing.
 *
 * revisions:
 *
 *	2010-08-01 rli: dorked with these comments.
 *
 *	2010-08-12 rli: disk drivers.
 *
 *	2013-06-01 rli: find the disks via bios_bios.
 *
 * formal parameters:
 *
 *	none.
 *
 * informal parameters:
 *
 *	- bios_xfer: Describes a disk transfer.
 *
 *	- bios_bios: the bios descriptor. holds a pointer to the disk
 *	  table.
 *
 * return value:
 *
 *	- 0: success.
 *
 *	- 1: failure.
 *
 ***/

unsigned short int bios_read( void )
{
  return bios_bios.disks[ bios_xfer.drive ]->read( &bios_xfer );
}

/***
 *
 * bios_write
 *
 * 	FUNCTION 14: WRITE SECTOR
 *
 *	This function is used to write 128 bytes of data from the
 *	currently selected DMA buffer to the currently selected sector,
 *	track, and disk. The parameter indicates whether the write is an
 *	ordinary write operation or whether there are special
 *	considerations.
 *
 *	The operation is passed off to the disk driver for further
 *	processing.
 *
 * revisions:
 *
 *	2010-08-01 rli: dorked with this comment block.
 *
 *	2010-08-12 rli: disk drivers.
 *
 *	2013-06-01 rli: find disk drivers via bios_bios.
 *
 * formal parameters:
 *
 *	- typecode: the type of the write.
 *
 * informal parameters:
 *
 *	- bios_xfer: Describes a disk transfer.
 *
 *	- bios_bios: the bios descriptor. holds a pointer to the disk
 *	  table.
 *
 * return value:
 *
 *	- 0: success.
 *
 *	- 1: error.
 *
 ***/

unsigned short int bios_write( unsigned short int typecode )
{
  return bios_bios.disks[ bios_xfer.drive ]->write( &bios_xfer, typecode );
}

/***
 *
 * bios_sectran
 *
 * 	FUNCTION 16: SECTOR TRANSLATE
 *
 *	This function performs logical-to-physical sector translation.
 *	The Sector Translate function receives a logical sector number.
 *	The logical sector number can range from 0 to the number of
 *	sectors per track - 1. Sector Translate also receives the
 *	address of teh translate table. The logical sector number is
 *	used as an index into the translate table. The resulting
 *	physical sector number is returned.
 *
 *	If the pointer to the translate table is null, implying that
 *	there is no translate table, the original sector number is
 *	returned. 
 *
 * revisions:
 *
 *	2010-08-01 rli: added this date tag.
 *
 * formal parameters:
 *
 *	- sector: logical sector number.
 *
 *	- table: pointer to sector number translation table.
 *
 * return value:
 *
 *	physical sector number.
 *
 ***/

unsigned short int bios_sectran( unsigned short int sector, 
  unsigned short int *table )
{
  if( table == 0 ) return sector;
  return table[ sector ];
}

/***
 *
 * FUNCTION 17: There is NO function 17
 *
 ***/

/***
 *
 * bios_getmrt
 *
 * 	FUNCTION 18: GET ADDRESS OF MEMORY REGION TABLE
 *
 *	This function returns the address of the Memory REgion Table
 *	(MRT). For compatibility with other CP/M system, CP/M-68K
 *	maintains a Memory Region Table. However, it contains only one
 *	region, the Transient Program Area (TPA). 
 *
 * revisions:
 *
 *	2010-08-01 rli: dorked with this common block.
 *
 *	2013-06-01 rli: bios_mrt renamed platform_mrt.
 *
 * formal parameters:
 *
 *	none.
 *
 * informal parameters:
 *
 *	- platfrom_mrt: The memory region table.
 *
 * return value:
 *
 *	- A pointer to the memory region table.
 *
 ***/

bios_mrt_t *bios_getmrt( void )
{
  return &platform_mrt;
}

/***
 *
 * bios_getiobyte
 *
 * 	FUNCTION 19: GET I/O BYTE
 *
 *	This function returns the currnet value of the logical to
 *	physical input/output device byte (I/O byte). This 8-bit value
 *	associates physical devices with CP/M-68k's four logical
 *	devices.
 *
 *	NOTE:	Even though this is a byte value, we are using
 *		word references. The upper byte should be zero.
 *
 * revisions:
 *
 *	2010-08-01 rli: added this date tag.
 *
 * formal parameters:
 *
 *	none.
 *
 * informal parameters:
 *
 *	- bios_iobyte: The I/O byte.
 *
 * return value:
 *
 *	a copy of the I/O byte.
 *
 ***/

unsigned short int bios_getiobyte( void )
{
  return bios_iobyte;
}

/***
 *
 * bios_setiobyte
 *
 * 	FUNCTION 20: SET I/O BYTE
 *
 *	This function sets the I/O byte to the specified value.
 *
 *	NOTE:	Even though this is a byte value, we are using word
 *		references. The upper byte should be zero.
 *
 * revisions:
 *
 *	2010-08-01 rli: dorked with this comment block.
 *
 * formal parameters:
 *
 *	- iobyte: The value to be stored in the I/O byte.
 *
 * informal parameters:
 *
 *	- bios_iobyte: The I/O byte.
 *
 * return value:
 *
 *	none.
 *
 ***/

void bios_setiobyte( unsigned short int iobyte )
{
  bios_iobyte = iobyte;
}

/***
 *
 * bios_flush
 *
 * 	FUNCTION 21: FLUSH BUFFERS
 *
 *	This function forces the contents of any disk buffers that have
 *	been modified to be written. It walks through the table of disk
 *	device drivers, telling each to flush its buffer.
 *
 * revisions:
 *
 *	2010-08-01 rli: added this date tag.
 *
 *	2010-08-22 rli: pass drive number to driver.
 *
 *	2013-06-01 rli: find the disks via bios_bios.
 *
 * formal parameters:
 *
 *	none.
 *
 * informal parameters:
 *
 *	- bios_bios: the bios descriptor. holds a pointer to the disk
 *	  table.
 *
 * return value:
 *
 *	A bitwise OR of all of the driver's returned status. Expected
 *	values are:
 *
 *	- 0: success.
 *
 *	- 0xffff: failure.
 *
 ***/

unsigned short int bios_flush( void )
{
  unsigned short int result = 0;
  unsigned int i;

  for( i = 0; i < 16; i++ )
    result |= bios_bios.disks[ i ]->flush( i );

  return result;
}

/***
 *
 * bios_setexc
 *
 * 	FUNCTION 22: SET EXCEPTION HANDLE ADDRESS
 *
 *	This function sets the specified exception vector such that it
 *	invokes the specified handler. The previous vector value is
 *	returned. Unlike the BDOS Set Exception Vector Function, this
 *	BIOS function sets any exception vector.
 *
 * revisions:
 *
 *	2010-08-01 rli: added this date tag. currently, we don't support
 *	  setting any vectors.
 *
 *	2013-06-04 rli: add an optional platform-specific callout.
 *
 * formal parameters:
 *
 *	- vector: The number of the vector to be set.
 *
 *	- handler: A pointer to the routine that is to be invoked.
 *
 * informal parameters:
 *
 *	- config_hassetexc_c: If this symbol is defined, a platform
 *	  exception setter is called.
 *
 * return value:
 *
 *	- The address of the previous handler function for the
 *	  exception, or NULL if there was no previous handler.
 *
 ***/

void *bios_setexc( unsigned short int vector, void *handler )
{
#ifdef config_hassetexc_c
  return platform_setexc( vector, handler );
#else
  return 0;
#endif
}

/***
 *
 * bios_getbios
 *
 *	This function returns a pointer to a structure describing the
 *	BIOS.
 *
 * revisions:
 *
 *	2010-08-22 rli: original version.
 *
 *	2013-05-30 rli: now returns a pointer to a generic bios
 *	  descriptor. programs that know about modubios will know
 *	  that it's really a modubios descriptor.
 *
 * formal parameters:
 *
 *	none.
 *
 * informal parameters:
 *
 *	- bios_bios: The structure describing the BIOS.
 * 
 * return value:
 *
 *	- A pointer to bios_bios.
 *
 ***/

bios_descriptor_t *bios_getbios( void )
{
  return (bios_descriptor_t *)&bios_bios;
}

