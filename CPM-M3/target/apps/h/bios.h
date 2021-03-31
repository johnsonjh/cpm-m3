/**********************************************************************
 *
 * file: bios.h
 *
 *	This file contains declarations used by the BIOS.
 *
 * revisions:
 *
 *	2010-08-03 rli: original version.
 *
 *	2010-08-16 rli: delete TPA constants; switched to symbols
 *	  defined by the linker.
 *
 *	2013-04-25 rli: Visual C packing pragma
 *
 *	2013-04-29 rli: back to gnu c packing. KEIL doesn't like initializers
 *	  for structures with a flexible array, so set the size of the
 *	  region array in bios_mrt_t to 1 for now. 
 *
 *	2013-05-28 rli: moved BIOS symbols here. added a generic BIOS
 *	  descriptor type and a prototype for bios_getbios.
 *
 **********************************************************************/

#ifndef bios_h_included
#define bios_h_included

/******************
 *
 *	CONSTANTS
 *
 ******************/

/***
 *
 * bios_*_c
 *
 *	Constants for the BIOS functions that can be called via the
 *	BDOS Call BIOS function (BDOS function 50).
 *
 *	The bios_getbios_c call is unique to this port. It returns a
 *	pointer to a structure that identifies the BIOS and may provide
 *	additional unique to the BIOS.
 *
 * revisions:
 *
 *	2013-05-03 rli: original version.
 *
 *	2013-05-28 rli: moved here.
 *
 ***/

#define bios_coldboot_c   0
#define bios_warmboot_c   1
#define bios_constat_c    2
#define bios_conin_c      3
#define bios_conout_c     4
#define bios_lstout_c     5
#define bios_punout_c     6 /* aka auxout */
#define bios_rdrin_c      7 /* aka auxin */
#define bios_home_c       8
#define bios_seldsk_c     9
#define bios_settrk_c    10
#define bios_setsec_c    11
#define bios_setdma_c    12
#define bios_read_c      13
#define bios_write_c     14
#define bios_liststat_c  15
#define bios_sectrn_c    16
                            /* 17: set dma segment address (cp/m86) */
#define bios_getseg_c    18
#define bios_getiob_c    19
#define bios_setiob_c    20
#define bios_bflush_c    21
#define bios_setvec_c    22

#define bios_getbios_c  255 

/***
 *
 * bios_bootdrive_c
 *
 *	This constant indicates which drive should become the default
 *	drive when the system cold boots. It is zero-based; 0 = A, 
 *	1 = B, etc.
 *
 * revisions:
 *
 *	2010-08-03 rli: original version.
 *
 *	2010-08-17 rli: death to CamelCase.
 *
 ***/

#define bios_bootdrive_c 0

/***
 *
 * bios_bootuser_c
 *
 *	This constant indicates which user number should become active
 *	when the system cold boots.
 *
 * revisions:
 *
 *	2010-08-03 rli: original version.
 *
 *	2010-08-17 rli: death to CamelCase.
 *
 ***/

#define bios_bootuser_c 0

/******************
 *
 *	TYPES
 *
 ******************/

/***
 *
 * bios_descriptor_t
 *
 *	This type contains information that identifies the bios; The 
 *	bios_getbios call returns a pointer to a structure of this type.
 *
 *	A BIOS may enclose this structure within a larger structure that
 *	provides additional information or access to services supplied
 *	by the BIOS. Such additional information is BIOS-specific.
 *
 * revisions:
 *
 *	2013-05-28 rli: original version.
 *
 * fields:
 *
 *	- tag: Holds a magic number that identifies the BIOS. Each BIOS
 *	  is expected to supply a unique magic number.
 *
 *	- version: Holds a magic number that identifies the version of
 *	  the BIOS. A program may combine this with the tag to infer the
 *	  layout of the rest of the structure.
 *	  
 ***/

typedef struct bios_descriptor_s {
  unsigned int tag;
  unsigned int version;
} bios_descriptor_t;

/***
 *
 * bios_dpb_t
 *
 *	This is the disk parameter block. It describes a disk to
 *	the system.
 *
 * revisions:
 *
 *	2005-03-07 rli: Added these comments.
 *
 ***/

typedef struct bios_dpb_s {
  unsigned short int spt;		/* sectors per track */
  unsigned char bsh;			/* block shift factor */
  unsigned char blm;			/* block mask */
  unsigned char exm;			/* extent mask */
  unsigned char reserved1;
  unsigned short int dsm;		/* number of blocks in disk */
  unsigned short int drm;		/* number of directory entries */
  unsigned short int reserved2;		/* initial allocation vector in -80 */
  unsigned short int cks;		/* size of checksum vector */
  unsigned short int off;		/* reserved tracks */
} bios_dpb_t;

/***
 *
 * bios_dph_t
 *
 *	This is the disk parameter header. It contains pointers to the
 *	various data structures needed to run a disk.
 *
 * revisions:
 *
 *	2005-03-07 rli: original version.
 *
 *	2010-08-22 rli: added another spare word for alignment.
 *
 *	2010-08-24 rli: decided to pack the structure rather than
 *	  add a word; it is publically visible, after all.
 *
 *	2013-04-25 rli: visual c packing
 *
 *	2103-04-29 rli: back to gnu c packing.
 *
 ***/

typedef struct bios_dph_s {
  unsigned short *xlt;			/* sector translation table address */
  unsigned short int scratch[ 3 ];	/* scratch words used by BIOS */
  unsigned char *dirbuf;		/* scratch sector for directory use */
  bios_dpb_t *dpb;			/* address of disk parameter block */
  unsigned char *csv;			/* checksum vector */
  unsigned char *alv;			/* allocation vector */
} __attribute__((packed)) bios_dph_t;

/***
 *
 * bios_mrt_t
 *
 *	This is the memory region table. It describes the RAM to the
 *	operating system.
 *
 *	We're reserving a page below the start of the operating system
 *	to hold the initial stack.
 *
 * revisions:
 *
 *	2005-03-07 rli: original version.
 *
 *	2010-08-16 rli: regions array.
 *
 *	2010-08-22 rli: packed attribute.
 *
 *	2013-04-25 rli: visual c packing.
 *
 *	2013-04-29 rli: back to gnu c packing. set array size of regions
 *	  field to 1 because KEIL doesn't like initializers for flexible
 *	  arrays. KEIL also wants regions to be packed because the
 *	  enclosing structure is packed.
 *
 * fields:
 *
 *	- count: Indicates how many regions are present in the table.
 *
 *	- regions: This array contains a description of each region.
 *	  Each entry contains:
 *
 *	  - base: The base address of the region.
 *
 *	  - length: The size, in bytes, of the region; base+length is
 *	    the address of the byte immediately following the region.
 *
 ***/

typedef struct bios_mrt_s {
  unsigned short int count;		/* number of regions in table */
  struct {
    void *base;				/* base address of this region */
    unsigned long int length;		/* length of this region */
  } __attribute__((packed)) regions[ 1 ];
} __attribute__((packed)) bios_mrt_t;

/*******************
 *
 *	GLOBALS
 *
 *******************/

/***
 *
 * copyrt
 *
 *	This string contains the Digital Research copyright message.
 *	It's defined in bdosmisc.c
 *
 * revisions:
 *
 *	2010-08-03 rli: original version.
 *
 ***/

extern char *copyrt;

/*******************
 *
 *	PROTOTYPES
 *
 *******************/

void bios_cboot( void );
void bios_wboot( void );
unsigned short int bios_const( void );
unsigned char bios_conin( void );
void bios_conout( unsigned char victim );
bios_descriptor_t *bios_getbios( void );
void bios_list( unsigned char victim );
void bios_punch( unsigned char victim );
unsigned char bios_reader( void );
void bios_home( void );
bios_dph_t *bios_seldsk( unsigned char drive, unsigned char logged );
void bios_settrk( unsigned short int track );
void bios_setsec( unsigned short int vector );
void bios_setdma( void *dmaaddress );
unsigned short int bios_read( void );
unsigned short int bios_write( unsigned short int typecode );
unsigned short int bios_listst( void );
unsigned short int bios_sectran( unsigned short int sector,
  unsigned short int *table );
bios_mrt_t *bios_getmrt( void );
unsigned short int bios_getiobyte( void );
void bios_setiobyte( unsigned short int iobyte );
unsigned short int bios_flush( void );
void *bios_setexc( unsigned short int vector, void *handler );

#endif /* ndef bios_h_included */
