/*****************************************************************
 *
 * file: cpm.h
 *
 *	This file declares structures needed to communicate with the
 *	operating system.
 *
 * revisions:
 *
 *	2010-08-02 rli: original version.
 *
 *	2010-08-28 rli: switch to CP/M-68K's layout of bpb. tpa
 *	  descriptor.
 *
 *	2011-03-06 rli: Added parameter to entry point in the basepage
 *	  to make it clear a loaded program is passed a pointer to the
 *	  basepage.
 *
 *	2013-04-25 rli: visual c packing instead of gnu c
 *
 *	2013-04-29 rli: back to gnu c packing.
 *
 *	2013-05-02 rli: fixed a couple of comments in the base page
 *	  description.
 *
 *	2013-05-03 rli: added constants for BDOS and BIOS functions.
 *
 *	2013-05-27 rli: moved bios constants to bios.h
 *
 *	2014-01-01 rli: trim the space reserved for the base page to
 *	  256 bytes to comport with tradition and the CP/M-68K manual,
 *	  but mostly the CP/M-68K manual. This only affects the comments.
 *
 ****************************************************************/

#ifndef cpm_h_included
#define cpm_h_included

/**********************
 *
 *	CONSTANTS
 *
 **********************/

/***
 *
 * bdos_*_c
 *
 *	Constants for the BDOS function numbers.
 *
 * revisions:
 *
 *	2013-05-03 rli: original version. moved EnterProgram so it
 *	  would not conflict with CP/Net functions.
 *
 ***/

#define bdos_warmboot_c             0
#define bdos_consoleinput_c         1
#define bdos_consoleoutput_c        2
#define bdos_readerinput_c          3
#define bdos_punchoutput_c          4
#define bdos_listoutput_c           5
#define bdos_rawconsoleio_c         6
#define bdos_getiobyte_c            7
#define bdos_setiobyte_c            8
#define bdos_printline_c            9
#define bdos_readline_c            10
#define bdos_consolestatus_c       11
#define bdos_version_c             12
#define bdos_resetdisks_c          13
#define bdos_selectdisk_c          14
#define bdos_openfile_c            15
#define bdos_closefile_c           16
#define bdos_searchfirst_c         17
#define bdos_searchnext_c          18
#define bdos_deletefile_c          19
#define bdos_readsequential_c      20
#define bdos_writesequential_c     21
#define bdos_createfile_c          22
#define bdos_renamefile_c          23
#define bdos_getloginvector_c      24
#define bdos_getcurrentdisk_c      25
#define bdos_setdmaaddress_c       26
                     /* 27: get allocation vector */
#define bdos_setreadonly_c         28
#define bdos_getreadonly_c         29
#define bdos_setfileattributes_c   30
#define bdos_getdiskparameters_c   31
#define bdos_getsetusernumber_c    32
#define bdos_readrandom_c          33
#define bdos_writerandom_c         34
#define bdos_getfilesize_c         35
#define bdos_setrandomrecord_c     36
#define bdos_resetdrive_c          37
                     /* 38: access drive (mp/m) */
                     /* 39: free drive (mp/m) */
#define bdos_writerandomwithfill_c 40
                     /* 41: test and write record (mp/m) */
                     /* 42: lock record (mp/m) */
                     /* 43: unlock record (mp/m) */
                     /* 44: set multi-sector count (mp/m) */
                     /* 45: set bdos error mode (mp/m) */
#define bdos_getdiskfreespace_c    46
#define bdos_chain_c               47
#define bdos_flushbuffers_c        48
                     /* 49: get/set sys ctrl blk (cp/m3) */
#define bdos_callbios_c            50
                     /* 51: set dma base (mp/m86) */
                     /* 52: get dma base (mp/m86) */
                     /* 53: get max mem (mp/m86) */
                     /* 54: get abs max (mp/m86) */
                     /* 55: alloc mem (mp/m86) */
                     /* 56: alloc abs max (mp/m86) */
                     /* 57: free mem (mp/m86) */
                     /* 58: free all mem (mp/m86) */
#define bdos_loadprogram_c         59
                     /* 60: call rsx (cp/m3) */
#define bdos_setexception_c        61
                     /* 62: unknown */
#define bdos_getsettpalimits_c     63
                     /* 64: login (cp/net) */
                     /* 65: logoff (cp/net) */
                     /* 66: set network message (cp/net) */
                     /* 67: receive network message (cp/net) */
                     /* 68: get network status (cp/net) */
                     /* 69: get configuration table addr (cp/net) */
                     /* 70: set compatibility attributes (cp/net) */
                     /* 71: get server configuration table addr (cp/net) */

                     /* 98: free blocks (cp/m3) */
                     /* 99: truncate file (cp/m3,ccp/m86) */
                     /* 100: set directory label (mp/m) */
                     /* 101: return directory label (mp/m) */
                     /* 102: read file xfcb (mp/m) */
                     /* 103: write file xfcb (mp/m) */
                     /* 104: set date and time (mp/m) */
                     /* 105: get date and time (mp/m) */
                     /* 106: set default password (mp/m) */
                     /* 107: get serial number (mp/m) */
                     /* 108: get/set prog ret code (cp/m3) */
                     /* 109: get/set cons mode (cp/m3,ccp/m86) */
                     /* 110: get/set delimiter (cp/m3,ccp/m86) */
                     /* 111: print block (cp/m3,ccp/m86) */
                     /* 112: list block (cp/m3,ccp/m86) */
                     /* 128: absolute mem request (mp/m) */
                     /* 129: relocatable mem req (mp/m) */
                     /* 130: memory free (mp/m) */
                     /* 131: poll (mp/m) */
                     /* 132: flag wait (mp/m) */
                     /* 133: flag set (mp/m) */
                     /* 134: make queue (mp/m) */
                     /* 135: open queue (mp/m) */
                     /* 136: delete queue (mp/m) */
                     /* 137: read queue (mp/m) */
                     /* 138: conditional rd queue (mp/m) */
                     /* 139: write queue (mp/m) */
                     /* 140: conditional wr queue (mp/m) */
                     /* 141: delay (mp/m) */
                     /* 142: dispatch (mp/m) */
                     /* 143: terminate process (mp/m) */
                     /* 144: create process (mp/m) */
                     /* 145: set priority (mp/m) */
                     /* 146: attach console (mp/m) */
                     /* 147: detach console (mp/m) */
                     /* 148: set console (mp/m) */
                     /* 149: assign console (mp/m) */
                     /* 150: send cli command (mp/m) */
                     /* 151: call res sys proc (mp/m) */
                     /* 152: parse filename (mp/m) */
                     /* 153: get console number (mp/m) */
                     /* 154: get system data address (mp/m) */
                     /* 155: get date and time (mp/m) */
                     /* 156: get PD addr (mp/m) */
                     /* 157: abort specific process (mp/m) */
                     /* 158: attach list (mp/m) */
                     /* 159: detach list (mp/m) */
                     /* 160: set list (mp/m) */
                     /* 161: conditional attach list (mp/m) */
                     /* 162: conditional attach cons (mp/m) */
                     /* 163: get NIPM version (mp/m) */ 
                     /* 164: get list number (mp/m) */

#define bdos_enterprogram_c       255 /* ARM bios */

/***
 *
 * cpm_iobyte_*
 *
 *	These constants describe the IOBYTE, which allows redirection of
 *	the character devices.
 *
 * revisions:
 *
 *	2010-08-07 rli: original version.
 *
 * values:
 *
 *	- cpm_iobyte_con_m: This bitmask extracts the field that
 *	  controls which device is being used for the console.
 *
 *	- cpm_iobyte_con_s: The size, in bits, of the console field.
 *
 *	- cpm_iobyte_con_v: The number of the least-signficant bit in
 *	  the console field.
 *
 *	- cpm_iobyte_con_tty_c: This value in the console field
 *	  indicates that TTY: should be used for console I/O.
 *
 *	- cpm_iobyte_con_crt_c: This value in the console field
 *	  indicates that CRT: should be used for console I/O.
 *
 *	- cpm_iobyte_con_bat_c: This value in the console field
 *	  indicates that BAT: should be used for console I/O.
 *
 *	- cpm_iobyte_con_uc1_c: This value in the console field
 *	  indicates that UC1: should be used for console I/O.
 *
 *	- cpm_iobyte_rdr_m: This bitmask extracts the field that
 *	  controls which device is being used for the reader.
 *
 *	- cpm_iobyte_rdr_s: The size, in bits, of the reader field.
 *
 *	- cpm_iobyte_rdr_v: The number of the least-signficant bit in
 *	  the reader field.
 *
 *	- cpm_iobyte_rdr_tty_c: This value in the reader field indicates
 *	  that TTY: should be used for the reader.
 *
 *	- cpm_iobyte_rdr_ptr_c: This value in the reader field indicates
 *	  that PTR: should be used for the reader.
 *
 *	- cpm_iobyte_rdr_ur1_c: This value in the reader field indicates
 *	  that UR1: should be used for the reader.
 *
 *	- cpm_iobyte_rdr_ur2_c: this value in the reader field indicates
 *	  that UR2: should be used for the reader.
 *
 *	- cpm_iobyte_pun_m: This bitmask extracts the field that
 *	  controls which device is used for the punch.
 *
 *	- cpm_iobyte_pun_s: The size, in bits, of the punch field.
 *
 *	- cpm_iobyte_pun_v: The number of the first bit in the punch
 *	  field.
 *
 *	- cpm_iobyte_pun_tty_c: This value in the punch field indicates
 *	  that TTY: should be used for the punch.
 *
 *	- cpm_iobyte_pun_ptp_c: This value in the punch field indicates
 *	  that PDP: should be used for the punch.
 *
 *	- cpm_iobyte_pun_up1_c: This value in the punch field indicates
 *	  that UP1: should be used for the punch.
 *
 *	- cpm_iobyte_pun_up2_c: This value in the punch field indicates
 *	  that UP2: should be used for the punch.
 *
 *	- cpm_iobyte_lst_m: This bitmask extracts the field that 
 *	  controls which device is used for the printer.
 *
 *	- cpm_iobyte_lst_s: The size, in bits, of the list field.
 *
 *	- cpm_iobyte_lst_v: The number of the first bit in the list
 *	  field.
 *
 *	- cpm_iobyte_lst_tty_c: This value in the list field indicates
 *	  that TTY: should be used for the list device.
 *
 *	- cpm_iobyte_lst_crt_c: This value in the list field indicates
 *	  that CRT: should be used for the list device.
 *
 *	- cpm_iobyte_lst_lpt_c: This value in the list field indicates
 *	  that LPT: should be used for the list device.
 *
 *	- cpm_iobyte_lst_ul1_c: This value in the list field indicates
 *	  that UL1: should be used for the list device.
 *
 ***/

#define cpm_iobyte_con_m 0x0003
#define cpm_iobyte_con_s 2
#define cpm_iobyte_con_v 0
#define cpm_iobyte_con_tty_c 0
#define cpm_iobyte_con_crt_c 1
#define cpm_iobyte_con_bat_c 2
#define cpm_iobyte_con_uc1_c 3

#define cpm_iobyte_rdr_m 0x000c
#define cpm_iobyte_rdr_s 2
#define cpm_iobyte_rdr_v 2
#define cpm_iobyte_rdr_tty_c 0
#define cpm_iobyte_rdr_ptr_c 1
#define cpm_iobyte_rdr_ur1_c 2
#define cpm_iobyte_rdr_ur2_c 3

#define cpm_iobyte_pun_m 0x0030
#define cpm_iobyte_pun_s 2
#define cpm_iobyte_pun_v 4
#define cpm_iobyte_pun_tty_c 0
#define cpm_iobyte_pun_ptp_c 1
#define cpm_iobyte_pun_up1_c 2
#define cpm_iobyte_pun_up2_c 3

#define cpm_iobyte_lst_m 0x00c0
#define cpm_iobyte_lst_s 2
#define cpm_iobyte_lst_v 6
#define cpm_iobyte_lst_tty_c 0
#define cpm_iobyte_lst_crt_c 1
#define cpm_iobyte_lst_lpt_c 2
#define cpm_iobyte_lst_ul1_c 3

/***
 *
 * cpm_tpa_*
 *
 *	These constants are used with function 63 to get and set the 
 *	TPA limits.
 *
 * revisions:
 *
 *	2010-08-28 rli: original version.
 *
 * values:
 *
 *	- cpm_tpa_set_m: When this bit is set in the flags word, the
 *	  TPA limits are set.
 *
 *	- cpm_tpa_permanent_m: When this bit set in the flags word
 *	  along with cpm_tpa_set_m, the permanent TPA limits are set.
 *
 ***/

#define cpm_tpa_set_m       1
#define cpm_tpa_permanent_m 2

/**********************
 *
 *	TYPES
 *
 **********************/

/***
 *
 * cpm_fcb_t
 *
 *	The file control block. This type is used to describe a file to
 *	the BDOS. There are two variants:
 *
 *	- The 33-byte sequential FCB.
 *	- The 36-byte random FCB.
 *
 *	This type describes the latter. If you need the former, just 
 *	don't touch the last three bytes.
 *
 * revisions:
 *
 *	2010-08-19 rli: original version.
 *
 * fields:
 *
 *	- dr: The drive code. This is one-based; zero refers to the
 *	  default drive, one to drive A:, etc.
 *
 *	- f: The filename.
 *
 *	- t: The filetype. The most-significant bit of each of these bytes
 *	  is used for a file attribute. Attributes are:
 *
 *	  - t[ 0 ]: Set indicates file is read-only.
 *	  - t[ 1 ]: Set indicates file is a system file.
 *	  - t[ 2 ]: Set indicates file has been archived.
 *
 *	- ex: Current extent number.
 *
 *	- s: Reserved for system use. Byte s[ 1 ] needs to be cleared
 *	  for Open, Make, and Search functions.
 *
 *	- rc: Record count. Reserved for system use.
 *
 *	- d: Disk allocation map. Reserved for system use.
 *
 *	- cr: Current record number within current extent. Used for
 *	  sequential operations.
 *
 *	- r: Random record position. Byte r[ 0 ] is the
 *	  least-significant byte.
 *
 ***/

typedef struct cpm_fcb_s {
  unsigned char dr;
  unsigned char f[ 8 ];
  unsigned char t[ 3 ];
  unsigned char ex;
  unsigned char s[ 2 ];
  unsigned char rc;
  unsigned char d[ 16 ];
  unsigned char cr;
  unsigned char r[ 3 ];
} cpm_fcb_t;

/***
 *
 * cpm_basepage_t
 *
 *	This type declares the base page, which contains information
 *	describing the environment in which a program loaded into the
 *	TPA finds itself. 
 *
 *	The fields contained herein are a mixture of those offered
 *	by CP/M-68K and by CP/M-80. Since addresses are 32-bits,
 *	we cannot simply clone the CP/M-80 base page. Since we are
 *	not using trap instructions to get to the BDOS, we have to
 *	provide a way for the program to find the BDOS entry point;
 *	in other words, we cannot simply clone the CP/M-68K base
 *	page.
 *
 *	Although I'm currently only supporting a .COM-style program
 *	structure, I'm keeping some of the more sophisticated fields
 *	offered by CP/M-68K in the hope that I'll eventually get around
 *	to doing a more sophisticated program loader.
 *
 *	This structure lies at the bottom of the TPA; the program
 *	is loaded above it. The program is also passed a pointer to
 *	the base page.
 *
 *	Although the base page occupies only 256 bytes, I'm reserving
 *	512 bytes for it at the bottom of the TPA; I don't recall why I
 *	decided to do that, probably to make it the size of a VAX page.
 *
 * revisions:
 *
 *	2005-03-04 rli: original attempt; version 0.
 *
 *	2005-04-01 rli: My bad; FCBs are 36 bytes long, not 35.
 *
 *	2005-04-09 rli: Botched reserved field before CommandTail.
 *
 *	2010-08-02 rli: genericized and trimmed to 256 bytes.
 *
 *	2010-08-17 rli: death to CamelCase and fields I'm not using.
 *
 *	2010-08-19 rli: ok, ok, i give in. switch to a layout that is
 *	  compatible with CP/M-68K and CP/M-8K.
 *
 *	2010-08-28 rli: bdos needs to return 32-bit int to support
 *	  direct BIOS calls.
 *
 *	2013-05-02 rli: fixed comments regarding typical placement
 *	  of the entry point.
 *
 * fields:
 *
 *	- tpabase: The base address of the TPA. This is also normally the
 *	  starting address of this structure.
 *
 *	- tpatop: The address immediately following the TPA. The
 *	  byte pointed to by this field is NOT in the TPA, but is
 *	  the byte immediately following the TPA. 
 *
 *	- textbase: The starting address of the .text section. This is
 *	  normally tpabase+0x100.
 *
 *	- textlen: The length, in bytes, of the .text section. 
 *
 *	  For the current .COM-style program loader, this is determined
 *	  by finding the end of the file. In other words, the program is
 *	  assumed to reside entirely within the text section.
 *
 *	- database: The starting address of the .data section.
 *
 *	  For the current .COM-style program loader, this is textbase.
 *
 *	- datalen: The length of the .data section.
 *
 *	  For the current .COM-style program loader, this is zero.
 *
 *	- bssbase: The starting address of the .bss section.
 *
 *	  For the current .COM-style program loader, this is textbase.
 *
 *	- bsslen: The length, in bytes, of the .bss section.
 *
 *	  For the current .COM-style program loader, this is zero.
 *
 *	- freelen: The size, in bytes, of the space between the top of
 *	  the program and the top of the TPA.
 *
 *	- loadedfrom: Drive from which the program was loaded. This is
 *	  one-based; 1-16 = A:-P:.
 *
 *	- reserved0: not used. this is padding for alignment.
 *
 *	- bdos: A pointer to the entry point to the BDOS. 
 *
 *	  CP/M-68K and CP/M-8K don't have this field.
 *
 *	- entry: A pointer to the program entry point.
 *
 *	  CP/M-68K and CP/M-8K don't have this field.
 *
 *	- initialsp: The value loaded into the stack pointer before the
 *	  program is entered. This normally tpatop.
 *
 *	  CP/M-68K and CP/M-8K don't have this field.
 *
 *	- reserved1: Not used.
 *
 *	- fcb2: The second FCB parsed from the command line.
 *
 *	- fcb1: The first FCB parsed from the command line.
 *
 *	- tail: The command tail and default DMA buffer.
 *
 ***/

typedef struct cpm_basepage_s {
  void *tpabase;			/* :0x000 */
  void *tpatop;				/* :0x004 */
  void *textbase;			/* :0x008 */
  unsigned int textlen;			/* :0x00c */
  void *database;			/* :0x010 */
  unsigned int datalen;			/* :0x014 */
  void *bssbase;			/* :0x018 */
  unsigned int bsslen;			/* :0x01c */
  unsigned int freelen;			/* :0x020 */
  unsigned char loadedfrom;		/* :0x024 */
  unsigned char reserved0[ 3 ];		/* :0x025 */
  unsigned int (*bdos)(			/* :0x028 */
    unsigned short int func,
    unsigned int parm );
  void (*entry)(  			/* :0x02c */
    struct cpm_basepage_s *basepage );
  void *initialsp;			/* :0x030 */
  unsigned int reserved1;               /* :0x034 */
  cpm_fcb_t fcb2;			/* :0x038 */
  cpm_fcb_t fcb1;  			/* :0x05c */
  unsigned char tail[ 128 ];		/* :0x080 */
} cpm_basepage_t;			/* :0x100 */

/***
 *
 * cpm_lpb_t
 *
 *	This data structure is used for the Program Load function, which
 *	is implemented in the BIOS. A pointer to a structure of this
 *	type is passed into the function. The information about the
 *	memory region into which the program is to be loaded is used
 *	by the function, which returns the information about the
 *	location of the base page and the entry point.
 *
 *	The current simple-minded .COM-style program loader fills in the
 *	following fields of the base page:
 *
 *	- tpabase, which receives the address of the base page.
 *	- tpatop, which receives the address of the byte following
 *	  the memory region into which the program was loaded.
 *	- textbase: Receives tpabase+0x100.
 *	- textlen: Receives the length, in bytes, of the executable
 *	  file.
 *	- database: Receives textbase.
 *	- datalen: Receives zero.
 *	- bssbase: Receives textbase.
 *	- bsslen: Receives zero.
 *	- freelen: Receives the size, in bytes, between the end of the
 *	  .text segment and the end of the TPA.
 *	- bdos: Receives a pointer to the BDOS entry point.
 *	- entry: Receives textbase.
 *	- initialsp: Receives tpatop.
 *	
 *	The remainder of base page initialization is the responsibility
 *	of the user.
 *
 * revisions:
 *
 *	2005-03-05 rli: original attempt.
 *
 *	2010-08-02 rli: genericized.
 *
 *	2010-08-16 rli: change types of TPA addresses for convenience
 *	  in pgmld.
 *
 *	2010-08-17 rli: death to CamelCase.
 *
 * fields:
 *
 *	- fcb: A pointer to the FCB from which the program is to be
 *	  read.
 *
 *	- tpabase: The base address of the memory region into which the
 *	  program is to be loaded. This is rounded up to the next VAX
 *	  page boundary before the program is loaded.
 *
 *	- tpatop: The address of the byte immediately following the
 *	  memory region into which the program is to be loaded. This
 *	  is rounded down to the previous VAX page boundary before the
 *	  program is loaded.
 *
 *	- basepage: Receives a pointer to the base page for the program.
 *
 *	- flags: Loader control flags. CP/M-68K defines the following flags:
 *
 *	  - Bit 0: The program is loaded into the bottom of the
 *	    TPA when clear and the top when set. 
 *
 *	  I'm not currently supporting this field.
 *
 ***/

typedef struct cpm_lpb_s {
  unsigned char *fcb;			/* :0x00 */
  unsigned int tpabase;			/* :0x04 */
  unsigned int tpatop;			/* :0x08 */
  cpm_basepage_t *basepage;		/* :0x0c */
  unsigned int flags;			/* :0x10 */
} cpm_lpb_t;


/***
 *
 * cpm_bpb_t
 *
 *	A pointer to a structure of this type is passed to BDOS function
 *	50, DIRECT BIOS CALL. It provides the information needed to call
 *	a BIOS function, to wit: the function number and a parameter to
 *	be passed to the function.
 *
 * revisions:
 *
 *	2005-03-05 rli: original attempt.
 *
 *	2010-08-02 rli: genericized.
 *
 *	2010-08-17 rli: death to CamelCase.
 *
 *	2010-08-28 rli: switched to CP/M-68K layout of BPB so I can
 *	  point to that manual in my own. yeah, that means the
 *	  parameters aren't aligned.
 *
 * fields:
 *
 *	- func: The number of the BIOS Function to be called.
 *
 *	- p1: The first parameter to be passed to that function, if
 *	  one is needed.
 *
 *	- p2: The second parameter to be passed to that
 *	  function, if one is needed.
 *
 ***/

typedef struct cpm_bpb_s {
  unsigned short int func;	/* :0x00 */
  unsigned long int p1;		/* :0x02 */
  unsigned long int p2;		/* :0x06 */
} __attribute__((packed)) cpm_bpb_t;

/***
 *
 * cpm_exehdr_t
 *
 *	An executable program begins with a structure of this type.
 *
 * revisions:
 *
 *	2010-08-02 rli: original version.
 *
 *	2010-08-15 rli: delete section addresses and sizes; initializing
 *	  the .bss section is the responsibility of the loaded program.
 *
 *	2010-08-17 rli: death to CamelCase.
 *
 *	2013-12-22 rli: replace version field with an entry pointer, to
 *	  allow Thumb architectures to force bit 0 on.
 *
 * fields:
 *
 *	- branch: Contains a branch instruction to the program entry point.
 *
 *	- tag: Contains a magic number that identifies the architecture
 *	  for which this program is meant.
 *
 *	- region: The address at which the image wants to be loaded;
 *	  remember that a region starts with the base page, for which
 *	  the image needs to leave space. This is intended to allow a
 *	  system with multiple memory regions (say, a small, fast one
 *	  attached right to the CPU and a large, slow one out on some
 *	  I/O bus somewhere) to select the appropriate region into which
 *	  to load the image.
 *
 *	- entry: Points to the image entry point.
 *
 ***/

typedef struct cpm_exehdr_s {
  unsigned int branch;              /* :0x00 */
  unsigned int tag;                 /* :0x04 */
  unsigned int region;              /* :0x08 */
  void (*entry)( cpm_basepage_t *); /* :0x0c */
} cpm_exehdr_t;

/***
 *
 * cpm_tpa_t
 *
 *	This type is used by function 63 to describe the TPA. It
 *	contains pointers to the first byte in the TPA and the byte
 *	immediately following the TPA. It also contains flags that
 *	indicate whether the function is to get or set the limits and,
 *	if setting, whether those limits are to be permanent.
 *
 * revisions:
 *
 *	2010-08-28 rli: original version.
 *
 ***/

typedef struct cpm_tpa_s {
  unsigned short int flags;
  char *base;
  char *top;
} __attribute__((packed)) cpm_tpa_t;

#endif

