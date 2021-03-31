/***
 *
 * revisions:
 *
 *	2010-08-21 rli: kblks and tall were not initialized in display.
 *
 ***/

/****************************************************************************/
/*									    */
/*	                  S T A T   C O M M A N D                           */
/*	                  -----------------------                           */
/*	                                                                    */
/*	Common module for MP/M 2.0, MP/M-86 2.0, CP/M-80 2.2, CP/M-86 1.1   */
/*		    and Portable CP/M implementation (P-CP/M)		    */
/*									    */
/****************************************************************************/


/****************************************************************************/
/*									    */
/*            Copyright(C) 1975, 1976, 1977, 1978, 1979, 1980, 1981	    */
/*            Digital Research						    */
/*            Box 579							    */
/*            Pacific Grove, Ca						    */
/*            93950							    */
/*									    */
/*            Revised:							    */
/*              20 Jan  80  by Thomas Rolander				    */
/*              29 July 81  by Doug Huskey (for MP/M 2.0)		    */
/*              02 Sept 81  (for MP/M-86)				    */
/*              14 Nov  81  by Doug Huskey (for CP/M-86)		    */
/*	        21 Sept 82  by Zilog (translate to C)			    */
/*									    */
/*									    */
/* Modified 10/30/78 to fix the space computation			    */
/* Modified 01/28/79 to remove despool dependencies			    */
/* Modified 07/26/79 to operate under CP/M 2.0				    */
/*									    */
/****************************************************************************/

#include "copyrt.lit"

/****************************************************************************/
/*									    */
/*	Note:  In an attempt to have a common source for all DRI O.S.	    */
/*	shared utilities the version is tested and a number of conditionally*/
/*	executed statements have been added for compatibility between MP/M 2*/
/*	and CP/M.							    */
/*									    */
/****************************************************************************/


/****************************************************************************/
/* Generation instructions                                                  */
/****************************************************************************/

/* To be provided later for Z8000 version				    */

/****************************************************************************/
/*									    */
/*	 H O W   I T   W O R K S   A N D   W H A T   I T   D O E S	    */
/*       ---------------------------------------------------------	    */
/*									    */
/*									    */
/* 	STAT gives information about disk drives, files and devices.  It    */
/*	can also change the attributes of files and disk drives, and the    */
/*	assignment of logical devices to physical devices.  STAT knows      */
/*	little about the extended file control information and disk	    */
/*	labels available under MP/M and CP/M 3.0, but is still required     */
/*	under these operating systems because many of its functions are     */
/*	not duplicated by other, more recent, utilities such as SET and     */
/*	SHOW.								    */
/*									    */
/*	The command takes several forms:				    */
/*									    */
/*	STAT VAL:							    */
/*		Display valid forms of the STAT command (ie a summary of    */
/*		the information set out below)				    */
/*									    */
/*	STAT								    */
/*		Display status of all checked drives (that is, drives	    */
/*		which have been explicitly or implicitly referenced since   */
/*		the last warm boot), saying how much free space is on	    */
/*		each and whether it is read/write or read-only		    */
/*									    */
/*	STAT d:								    */
/*		Display status as above for a specified disk drive d:	    */
/*									    */
/*	STAT DSK:							    */
/*	STAT d:DSK:							    */
/*		Same as first two cases above, except that considerably     */
/*		more information is displayed: capacity in 128 byte records,*/
/*		capacity in Kbytes, total number of directory entries,	    */
/*		number used, number of records allocated per directory	    */
/*		entry (physical extent), number of records per block and    */
/*		per track and the number of reserved tracks		    */
/*									    */
/*	STAT d:=RO							    */
/*		Set a specified disk drive to read-only status.  It retains */
/*		this status until the next warm boot or until...	    */
/*									    */
/*	STAT d:=RW							    */
/*		Set a specified disk drive to read-write status.  Cancels   */
/*		a previous STAT d:=RO					    */
/*									    */
/*	STAT USR:							    */
/*	STAT d:USR:							    */
/*		Lists the users who own files on the named disk drive.  If  */
/*		no drive is named, the current drive is assumed		    */
/*									    */
/*	STAT DEV:							    */
/*		Display the current logical to physical device assignments  */
/*		(Not applicable to MP/M)				    */
/*									    */
/*	STAT log:=phy:							    */
/*		Assign logical device log: to physical device phy:, for	    */
/*		example STAT CON:=TTY: (Not applicable to MP/M)		    */
/*									    */
/*	STAT filespec							    */
/*	STAT filespec SIZE						    */
/*		Display detailed information about the file or files	    */
/*		referenced by filespec (filespec may be ambiguous or	    */
/*		unambiguous.)  Information is number of physical records,   */
/*		physical size in Kbytes, number of FCB's (directory	    */
/*		entries) used for this file and attributes.  If the word    */
/*		SIZE follows filespec, the logical record count is also     */
/*		printed.  This may be greater than the physical record	    */
/*		count if the file is sparce (not all logical records used.) */
/*									    */
/*	STAT filespec RO | RW | SYS | DIR				    */
/*		Sets or clears attributes of specified file or files.  RO   */
/*		(read-only) and RW (read/write) are mutually exclusive, as  */
/*		are SYS (system file - does not normally appear in	    */
/*		directory listings) and DIR (directory file - can be seen   */
/*		in directory listings.)  Thus, no more than two attributes  */
/*		may be specified on the same command line.  The order in    */
/*		which attributes are specified is not important.	    */
/*									    */
/*									    */
/*	To find out how this program implements these functions, it is best */
/*	to start at the end of this listing, which defines the top-level    */
/*	function, and work backwards through progressively lower-level	    */
/*	functions as and when it becomes necessary.			    */
/*									    */
/*	For historical reasons, much of the communication between functions */
/*	uses global variables rather than parameters.  The main things to   */
/*	remember are that all global variables are defined at the top of    */
/*	the program before any function definitions, and that all globals   */
/*	NEED to be global because they are referenced by more than one	    */
/*	function.  A cross reference listing of some sort is useful in	    */
/*	tracking interdependencies caused by global variable usage.	    */
/*									    */
/*					Dominic Dunlop, Zilog Inc.  10/20/82*/
/*									    */
/****************************************************************************/

/****************************************************************************/
/*									    */
/*	   E X T E R N A L   A N D   B D O S   I N T E R F A C E            */
/*	   -----------------------------------------------------            */
/*									    */
/****************************************************************************/

#include "portab.h"				/* Portable C definitions   */
#include "bdos.h"				/* CP/M, MP/M call def's    */
#include "basepage.h"				/* CP/M basepage structure  */

/****************************************************************************/
/*		        External data and functions                         */
/****************************************************************************/

extern	char		*sbrk();		/* Memory allocation	    */
extern	char		*_break;		/* Top of allocated memory  */

struct 	fcbtab		*fcb;			/* Ptr to 1st basepage FCB  */
char			*buff;			/* Ptr to basepage DMA buff */


/****************************************************************************/
/*			    Version dependencies                            */
/****************************************************************************/

						/* Version number fields    */
#define	CPM		0x0000			/* Vanilla flavor CP/M	    */
#define MPM		0x1000			/* MP/M			    */
#define PCPM		0x2000			/* Portable CP/M	    */
#define ONE_X		0x00			/* Version 1.x		    */
#define TWO_X		0x20			/* Version 2.x		    */
#define THREE_X		0x30			/* Version 3.x		    */

#define VOID_GET_DPB(x)	(((x) & 0xff00) == PCPM)
#define HAS_GET_DFS(x)	((((x) & 0xff00) != CPM) || (((x) & 0xf0) >= THREE_X))
#define HAS_GSET_SCB(x)	(((x) & 0xf0) >= THREE_X)


/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

						/* Absolute value (beware   */
						/*   side effects!)	    */
#define abs(a)	(((a) >= 0) ? (a) : -(a))

#define SAFETY		0x400			/* Stack expansion space    */
						/*   after memory allocation*/
#define WIDTH		72			/* Default console width    */
#define	SPKSHF		3			/* log base 2 of sectors/K  */
#define DELETED		0xe5			/* Deleted file mark	    */
#define T_SIZE		4			/* Size of command token    */
#define NSIZE		(sizeof fcb->fname)	/* Size of fname	    */
#define	FNAM		(NSIZE + sizeof fcb->ftype)/* " + size of ftype	    */
#define	ROFILE		ftype[0]		/* Read-only file	    */
#define	SYSFILE		ftype[1]		/* "System" (invisible) file*/
#define	ARCHIV		ftype[2]		/* Archived  file	    */
#define HAS_XFCB	s1			/* Has extended FCB	    */
#define	ATTRB1		fname[0]		/* Attribute F1' 	    */
#define	ATTRB2		fname[1]		/* Attribute F2' 	    */
#define	ATTRB3		fname[2]		/* Attribute F3' 	    */
#define	ATTRB4		fname[3]		/* Attribute F4' 	    */


/****************************************************************************/
/* Miscellaneous global data                                                */
/****************************************************************************/

char			cdisk;			/* Current disk		    */
char			token[T_SIZE];		/* Parsed cmd line token    */
char			user_code;		/* Current user code	    */
BOOLEAN			sizeset	= FALSE;	/* TRUE if printing size    */
BOOLEAN			set_attribute = FALSE;	/* TRUE if setting attribute*/
BOOLEAN			error_free = TRUE;	/* No duplicate block error */  
BOOLEAN			word_blks;		/* FCB block addr's 16 bits */
int			kpb;			/* Kbytes per disk block    */
int			scase1, scase2;		/* Attributes required	    */
UWORD			ver;			/* OS version number	    */
UWORD			dcnt;			/* Current directory code   */
UWORD			rodisk;			/* Read only disk vector    */ 
UWORD			nfcbs  = 0;		/* Total number of FCB's    */

/****************************************************************************/
/* Data associated with disk allocation                                     */
/*									    */
/* NOTE: The local copy of the disk parameter block is used only in those   */
/*	 CP/M implementations where BDOS function 31 returns a copy of the  */
/*	 DPB.  Other implementations access the BIOS' own copy of the DPB.  */
/*									    */
/*	 The length of the undimensioned array ALLOC is fixed at runtime    */
/*	 according to the number of blocks on the disk being examined	    */
/****************************************************************************/

struct	dpbs		dpb;			/* Local copy of DPB	    */
struct	dpbs		*dpba = &dpb;		/* Ptr to disk parameter blk*/

BYTE			*alloc;			/* Disk map, one bit / block*/


/****************************************************************************/
/* Data associated with collecting and sorting filenames                    */
/*									    */
/* NOTE: The lengths of the two undimensioned arrays are fixed at runtime   */
/*	 according to the amount of free memory available		    */
/****************************************************************************/

UWORD			fcbn;			/* FCB's collected so far   */
UWORD			fcbmax;			/* Size of the arrays below */

struct	fcbhalf					/* First 16 bytes of FCB:   */
	{					/*   used in sorting files  */
		BYTE	drive;			/*   and gathering data	    */
		BYTE	fname[8];
		BYTE	ftype[3];
		BYTE	extent;
		BYTE	s1;
		UWORD	kcnt;			/* Kilobyte count ) Not from*/
		UWORD	rcnt;			/* Record count   ) FCB     */
	} *fcbs;				/* Base for collecting FCB's*/

struct	fcbhalf		**finx;			/* Array of FCB addresses   */
struct	fcbhalf		*fcbsa;			/* Index into FCB's	    */
struct	fcbtab		*bfcba;			/* Ptr to directory entry   */

struct	block_no				/* FCB pointer to disk block*/
	{					/* May be either byte or    */
		BYTE	first;			/*   8080-ordered word	    */
		BYTE	second;
	};

/****************************************************************************/
/* Messages to suit (almost) every occasion                                 */
/* (all these messages are used more than once - messages used only once    */
/*  are coded in line)							    */
/****************************************************************************/

char	drivename[]	= " Drive ";
char	readonly[]	= "Read Only (RO)";
char	readwrite[]	= "Read Write (RW)";
char	entries[]	= "  Directory Entries";
char	filename[]	= "d:filename.typ";
char	use[]		= "Use: STAT ";
char	invalid[]	= "Invalid Assignment";
char	set_to[]	= " set to ";
char	record_msg[]	= "128 Byte Record";
char	sattrib[]	= "[RO] [RW] [SYS] or [DIR]";


/****************************************************************************/
/* Valid first tokens on command line                                       */
/****************************************************************************/

char	devl[]		= "CON:AXI:AXO:LST:DEV:VAL:USR:DSK:";

#define L_SIZE		((sizeof devl - 1) / T_SIZE)/* Number of entries    */
#define	OPT_USR		7			/* Positions (starting at 1)*/
#define OPT_DSK		8			/*   of some options	    */


/****************************************************************************/
/* Valid attributes for files, disks                                        */
/****************************************************************************/

char	attribl[]	= "RO  RW  SIZESYS DIR ";

#define A_SIZE		((sizeof attribl - 1) / T_SIZE)/* Number of entries */

#define OPT_RO		1			/* Positions of attributes  */
#define OPT_RW		2			/* NOTE: do not change this */
#define OPT_SIZE	3			/*   order: error checking  */
#define OPT_SYS		4			/*   depends on it	    */
#define OPT_DIR		5


/****************************************************************************/
/* Valid names for physical devices                                         */
/****************************************************************************/

char	devr[] =
	"TTY:CRT:BAT:UC1:TTY:PTR:UR1:UR2:TTY:PTP:UP1:UP2:TTY:CRT:LPT:UL1:";

#define P_SIZE		((sizeof devr - 1) / T_SIZE)/* Number of entries    */

/****************************************************************************/
/*									    */
/*	     	     L O W _ L E V E L   F U N C T I O N S                  */
/*	             -------------------------------------                  */
/*									    */
/****************************************************************************/


		/********************************/
		/*		 		*/
		/*          B L A N K S         */
		/*		 		*/
		/********************************/

VOID						/* Print b blanks on the    */
blanks(b)					/*   console		    */
int	b;
{
	while (b--)
        	_conout(' ');
}


		/********************************/
		/*		 		*/
		/*        P R I N T X           */
		/*		 		*/
		/********************************/

VOID						/* Print a null-terminated  */
printx(a)					/*   string on the console  */
char	*a;
{
        while (*a) 
        	_conout(*a++);
}
  
  
		/********************************/
		/*		 		*/
		/*         N E W _ L N          */
		/*		 		*/
		/********************************/

VOID						/* Send carriage-return,    */
new_ln()					/*   line-feed to console   */
{
	_conout('\n');
	_conout('\r');
}


		/********************************/
		/*		 		*/
		/*   T E S T _ K B D _ E S C    */
		/*		 		*/
		/********************************/

VOID						/* If a character has been  */
test_kbd_esc()					/*   entered at the console,*/
{						/*   discard it and exit    */
	if (_constat())				/*   from STAT at once	    */
	{ 
		_conin();			/* Read & discard character */
		new_ln();
		printx("* Aborted *");
		_exit(1);
	}
}


		/********************************/
		/*		 		*/
		/*           C R L F            */
		/*		 		*/
		/********************************/

VOID						/* Send CR - LF sequence    */
crlf()						/*   then check if user	    */
{						/*   wants to abort (has    */
    new_ln();					/*   hit any key)	    */
    test_kbd_esc();
}


		/********************************/
		/*		 		*/
		/*         P R I N T            */
		/*		 		*/
		/********************************/

VOID						/* Print a null-terminated  */
print(a)					/*   string on a new line   */
char	*a;
{
	crlf();
	printx(a);
}


		/********************************/
		/*		 		*/
		/*       C O L U M N S          */
		/*		 		*/
		/********************************/

int						/* Returns the number of    */
columns()					/*   print columns on the   */
{						/*   console (gives default */
	static struct scbpb	colpb =		/*   if not BDOS 3)	    */
	{
		0x1a,				/* (Gives offest of con_w)  */
		GET
	};

	if (HAS_GSET_SCB(ver))
#ifdef HILO					/* _gset_scb returns a word */
		return (_gset_scb(&colpb) >> 8);/* We want more significant */
#else						/*   byte on HILO machines, */
		return (_gset_scb(&colpb));	/*   less significant on    */
#endif						/*   LOHI		    */
	else
		return (WIDTH);
}
		

		/********************************/
		/*		 		*/
		/*        S E L E C T           */
		/*		 		*/
		/********************************/

VOID						/* Select the disk d, making*/
select(d)					/*   it the current disk    */
int	d;
{
	rodisk = _get_ro();			/* Get current Read-Only vec*/
	cdisk = d;
	_sel_disk(d);
}


		/********************************/
		/*		 		*/
		/*       S E T _ K P B          */
		/*		 		*/
		/********************************/

VOID						/* Calculate the size of a  */
set_kpb()					/*   disk block in kilobytes*/
{						/* Also decide whether disk */
						/*   block addresses are 8  */
						/*   or 16 bits long	    */

/****************************************************************************/
/*									    */
/* We want kpb (kbytes per block) so that each time we find		    */
/*   a block address we can add kpb k to the kilobyte accumulator	    */
/*   for file size.  We derive kpb from bls - the base 2 logarithm of the   */
/*   number of 128-byte records per block - as follows:			    */
/*									    */
/*	 BLS  	      RECS/BLK	      K/BLK	    BLS - 3		    */
/*	  3               8             1              0		    */
/*	  4              16             2              1		    */
/*	  5              32             4              2		    */
/*	  6              64             8              3		    */
/*	  7             128            16              4		    */
/*									    */
/****************************************************************************/

	if (VOID_GET_DPB(ver))			/* Get DPB (just how depends*/
	{					/*   on CP/M implementation)*/
		_get_dpb(&dpb);
	}
	else
	{
		dpba = (struct dpbs *) _get_dpa();
	}
	kpb = 1 << (dpba->bls - SPKSHF); 	/* Calculate K per block    */
	word_blks = (dpba->mxa > 255);
}


		/********************************/
		/*		 		*/
		/*    S E L E C T _ D I S K     */
		/*		 		*/
		/********************************/

VOID						/* Select a disk and	    */
select_disk(d)					/*   calculate how many	    */
int	d;					/*   kbytes/block it has    */
{
	select(d);
	set_kpb();
}


		/********************************/
		/*		 		*/
		/*          C O U N T           */
		/*		 		*/
		/********************************/

UWORD						/* Returns the number of    */
count()						/*   KBYTES remaining on    */
{						/*   current disk	    */
	register UWORD	k, all_blks;
	register char	*all_vec, *all_end;
	LONG		maxall;

	if (HAS_GET_DFS(ver))			/* BDOS may do most of the  */
	{					/*   work for us	    */
		maxall = 0;
		_setdma(&maxall);
		_get_dfs(cdisk);
		return(maxall >> SPKSHF);	/* Convert from records to  */
						/*   K (3 places)	    */
	}
						/****************************/
						/*			    */
						/* BEWARE!  _get_alloc()    */
						/* does not work well on    */
						/* systems with bank-switch,*/
						/* segmented and/or protec- */
						/* ted memory.  Such systems*/
						/* MUST use _get_dfs() above*/
						/*			    */
						/****************************/

	all_vec = (char *) _get_alloc();	/* Find where allocation vec*/
	all_end = &all_vec[dpba->mxa / 8];	/*   starts and ends	    */

	for (all_blks = 0; all_vec <= all_end; all_vec++)
	{					/* Count up number of bits  */
		k = *all_vec;			/*   set in vector (use	    */
		do				/*   incrementing pointer   */
		{				/*   because quicker than   */
			all_blks += k & 1;	/*   incrementing index)    */
		} while (k >>= 1);		/* At end, we have count of */
	}					/*   allocated blocks	    */

	if ((maxall = dpba->mxa) <= all_blks)	/* If disk is full, return 0*/
		return (0);
	
	return (kpb * (maxall - all_blks + 1));	/* Not full: multiply blocks*/
}						/*   free by kbytes/block   */


		/********************************/
		/*		 		*/
		/*          F I L L             */
		/*		 		*/
		/********************************/

VOID						/* Fill the string s with   */
fill(s,f,c)					/*   the character f for c  */
register char	*s;				/*   positions		    */
register int	f;
register UWORD	c;
{
	while (c--)
		*s++ = f;
}

/****************************************************************************/
/*									    */
/*	            M E M O R Y   A L L O C A T I O N                       */
/*	            ---------------------------------                       */
/*									    */
/*	A word about how memory is allocated (a picture is worth a thousand,*/
/*	so it is rumored):						    */
/*									    */
/*	+-------------------------------+  <--- Address zero		    */
/*	|				|				    */
/*	|	     S T A T		|	Only this area is allocated */
/*	|				|	when STAT is initially	    */
/*	|  C O D E ,  D A T A,  B S S	|	loaded by CP/M (STAT is     */
/*	|				|	very approx 8k in length)   */
/*	|				|				    */
/*	+-------------------------------+  <--- alloc points here	    */
/*	|				|				    */
/*	|	DISK ALLOCATION MAP	|	Allocated by all_map()	    */
/*	|				|				    */
/*	+-------------------------------+  <--- finx points here	    */	/*	|  				|				    */
/*	|  ARRAY OF POINTERS INTO fcbs	|	Allocated by all_fcb	    */
/*	|				|				    */
/*	+-------------------------------+  <--- fcbs points here	    */
/*	|				|				    */
/*	| ARRAY OF FILE CONTROL BLOCKS	|	Allocated by all_fcb	    */
/*	|				|				    */
/*	+-------------------------------+				    */
/*	|				|				    */
/*	|      UNALLOCATED MEMORY	|				    */
/*									    */
/*	STAT only grows beyond its load size if asked to examine files.	    */
/*	The disk allocation map is typically very short.  The array of	    */
/*	pointers and the array of file control blocks take up between them  */
/*	the rest of available memory or enough to be able to accomodate the */
/*	maximum number of directory entries possible on the disk under	    */
/*	scrutiny, whichever is smaller.  This typically means that, taken   */ 
/*	together, they total about 60% of the length of a disk track.	    */
/*	Note that only 18 bytes (not 32 or 36) are stored for each file	    */
/*	(not for each extent.)						    */
/*									    */
/****************************************************************************/


		/********************************/
		/*		 		*/
		/*        A L L _ M A P         */
		/*		 		*/
		/********************************/

char *						/* Allocate memory to hold  */
all_map()					/*   disk allocation map    */
{						/* Delivers the address of  */
	register char 	*addr;			/*   the map		    */
						/* NOTE: must be called	    */
						/*   BEFORE all_fcb	    */

						/* Disk has mxa blocks.     */
						/* Take a bit for each	    */
	if ((int) (addr = sbrk(dpba->mxa / 8) + 1) == -1)
	{
		print("Insufficient Memory For Allocation Map");
		_exit(1);
	}
	fill(addr, 0, dpba->mxa / 8);		/* Clear space out	    */
	return (addr);
}


		/********************************/
		/*		 		*/
		/*        A L L _ F C B         */
		/*		 		*/
		/********************************/

UWORD						/* Allocate memory to hold  */
all_fcb()					/*   FCB's and FCB pointers */
{
	register UWORD	dimension;		/* Sets pointers to base of */
						/*   each array.  Returns   */
						/*   no. of array elements  */

						/* How much room between    */
						/*   start of free memory   */
						/*   and stack end (leave   */
						/*   a safety margin)?	    */

	dimension = ((UWORD) _base->freelen - SAFETY) /
		      (sizeof *finx + sizeof (struct fcbhalf));
	if (dimension == 0)
	{
		print("Insufficient Memory for FCB's");
		_exit(1);
	}
	dimension = (dpba->dmx + 1 < dimension ) ?/* We actually only need  */
		      dpba->dmx + 1 : dimension;/*   enough for maximum no. */
						/*   of directory entries   */

						/* Allocate the space	    */
	finx = (struct fcbhalf **) sbrk((sizeof *finx) * dimension);
	fcbs = fcbsa = (struct fcbhalf *)
		sbrk((sizeof (struct fcbhalf)) * dimension);

						/* Clear it out		    */
	fill((char *) finx, 0, dimension *
		      (sizeof *finx + sizeof (struct fcbhalf)));
	return (dimension);
}

/****************************************************************************/
/*									    */
/*	        C O M M A N D   L I N E   S C A N N I N G                   */
/*	        -----------------------------------------                   */
/*									    */
/****************************************************************************/


		/********************************/
		/*		 		*/
		/*            S C A N           */
		/*		 		*/
		/********************************/

VOID						/* Put the next input value */
scan()						/*   into the token accum-  */
{						/*   ulator		    */
	static int	ibp = 1;		/* Input buffer pointer	    */
	int		b, scandex;

	while (buff[ibp++] == ' ');		/* Skip leading spaces	    */

	if (buff[--ibp] == '[') 		/* Back up to first char    */
		ibp++ ;				/*   which is not ' ' or '['*/

	scandex = 0;				/* Initialize accum index   */
	fill(token, ' ', T_SIZE);		/*   & clear accumulator    */

	while ((b = buff[ibp]) > 1)		/* Get char: exit if 0 or 1 */
	{
		switch (b)			/* Does character terminate */
		{				/*   the token?		    */
		  case ' ':
		  case ',':
		  case ':':
		  case '[':
		  case '=':
			buff[ibp] = 1;		/* Yes: set termination	    */
			break;			/*   condition		    */
		
		  default:			/* No (unless it's a control*/
			if (b < ' ')		/*   character)		    */
				buff[ibp] = 1;
			else
				ibp++ ;		/* Point to next char	    */
		}
		
		switch (b)			/* Add char to accumulator  */
		{				/*   unless it's one of	    */
		  case '/':			/*   these		    */
		  case '_':
		  case ']':
		  case ',':
			break;
		
		  default:			/* Put next character of    */
			if (scandex < T_SIZE)	/*   token (which my be no  */
				token[scandex] = b;/*longer than 4 chars)   */
			scandex++;		/*   into accumulator	    */
		}
	}

	if (b != 0)  				/* At end of command line?  */
		ibp++;				/* No: bump pointer	    */
}


		/********************************/
		/*		 		*/
		/*   P A R S E _ A S S I G N    */
		/*		 		*/
		/********************************/

BOOLEAN						/* Parse an assignment into */
parse_assign()					/*   the accumulator	    */
{
	scan();					/* Get a token		    */
	if (token[0] != '=') 			/* Is it '='?		    */
		return (FALSE);			/* No: not an assignment    */
	scan(); 				/* Yes: get token to be	    */
	return (TRUE);				/*   assigned		    */
}


		/********************************/
		/*		 		*/
		/*     P A R S E _ N E X T      */
		/*		 		*/
		/********************************/

BOOLEAN						/* Parse next item into	    */
parse_next()					/*   accumulator	    */
{
	scan();
	if (token[0] == ' ')			/* Was it just a delimiter? */
	{
		scan();				/* Yes: try again	    */
		if (token[0] == ' ') 		/* Another delimiter (or    */
			return (FALSE);		/*   line end): give up	    */
	}
	return (TRUE);
}


		/********************************/
		/*		 		*/
		/*          M A T C H           */
		/*		 		*/
		/********************************/

int						/* Try to match 4-character */
match(va, vl)					/*   token in accumulator to*/
register char	*va;				/*   entry in table of vl   */
int	vl;					/*   entries at va	    */
{						/* Return index into table  */
						/*   (> 0) if match found   */
	register int	j, k, sync;		/*   else 0		    */
	BOOLEAN		found;

	j = 0;					/* j indexes devices table  */
	for (sync = 1; sync <= vl; sync++)	/* sync counts devices tried*/
	{
		found = TRUE;			/* Be optimistic!	    */
		for (k = 0; k < T_SIZE; ) 	/* Compare characters	    */
		{
			if ((va[j] == ' ') 	/* Don't attempt to match   */
			   && found)		/*   trailing blanks on	    */
				break;		/*   table entry - match OK */

			if (va[j++] != token[k++])/* Make comparison	    */
				found = FALSE;	/* It failed.  Stay in inner*/
		}				/*   loop to skip to next   */
						/*   possible value	    */
		if (found)			/* Success!		    */
			return (sync);
	}
	return (0);				/* Failure		    */
}

/****************************************************************************/
/*									    */
/*	             D I S P L A Y   F U N C T I O N S                      */
/*	             ---------------------------------                      */
/*									    */
/****************************************************************************/


		/********************************/
		/*		 		*/
		/*       P D E C I M A L        */
		/*		 		*/
		/********************************/

VOID						/* Print long unsigned value*/
pdecimal(v, prec, zerosup)			/*   to set precision	    */
						/*   1 = 1 place, 10 = 2    */
						/*   places etc.)	    */
UWORD	v;					/* Value to print	    */
UWORD	prec;					/* Precision		    */
BOOLEAN	zerosup;				/* Zero suppression flag    */
{
	int	d;				/* Current decimal digit    */

	while (prec != 0)
	{
		d = v / prec;			/* Get next digit	    */
		v %= prec;			/* Get remainder back to v  */
		if (((prec /= 10) != 0)		/* Is this a supressed	    */
		   && zerosup) {		/*   leading zero?	    */
			if (d == 0)
				_conout(' ');	/* Yes: print space	    */
			else
			{			/* No: print digit	    */
				zerosup = FALSE;
				_conout('0' + d);
			}
		}
		else	_conout('0' + d);
	}
}


		/********************************/
		/*		 		*/
		/*        P _  L O N G          */
		/*		 		*/
		/********************************/

VOID						/* Print an unsigned long   */
p_long(value)					/*   value to seven digit   */
LONG	value;					/*   precision, with commas */
{						/*   every three digits     */

	UWORD	thous[4];			/* 3-digit chunks of value  */
	register int j;
	register int zerosup;

	zerosup = TRUE;
	for (j = sizeof thous / sizeof (UWORD); --j >= 0;)
						/* Break the number up into */
	{					/*   3 digit chunks (most   */
		thous[j] = value % 1000;	/*   significant thous[0])  */
		value /= 1000;
	}
	for (j = 0; j < (sizeof thous / sizeof (UWORD)); j++)
						/* Print out chunks, most   */
	{					/*   significant first	    */
		switch(j)			/* Handle special cases (all*/
		{				/*   cases are special!)    */
		  case 0:
			break;			/* Billions: don't print    */

		  case 1:			/* Millions: just one digit */
			if (thous[j] == 0)	/*   or spaces if zero	    */
				printx("  ");
			else
			{
				pdecimal(thous[j], 1, zerosup);
				_conout(',');
				zerosup = FALSE;
			}
			break;
		
		  case ((sizeof thous / sizeof (UWORD)) - 1):
						/* Hundreds, tens, units:   */	
				pdecimal(thous[j], 100, zerosup);
						/*  always print   */
				zerosup = FALSE;
				break;

		  default:			/* The rest: print zero as  */
			if (thous[j])		/*   spaces, follow digits  */
			{			/*   with comma		    */
				pdecimal(thous[j], 100, zerosup);
				_conout(',');
				zerosup = FALSE;
			}
			else
				printx("    ");
		}
	}
}


		/********************************/
		/*		 		*/
		/*         P _ U N L            */
		/*		 		*/
		/********************************/

VOID						/* Print unsigned long value*/
p_unl(value)					/*   on a new line followed */
LONG	value;					/*   by ": "		    */
{
	crlf();
	p_long(value);
	printx(": ");
}
	

		/********************************/
		/*		 		*/
		/*        S H O W _ D V         */
		/*     S H O W _ D R I V E      */
		/*		 		*/
		/********************************/

VOID						/* Display name (A: - P:)   */
show_dv()					/*   of current drive	    */
{
	_conout(cdisk + 'A');
	_conout(':');
}

VOID						/* Same as above, followed  */
show_drive()					/*   by space		    */
{
	show_dv();
	_conout(' ');
}


		/********************************/
		/*		 		*/
		/*        S H O W _ U S R       */
		/*		 		*/
		/********************************/

VOID						/* Display current user	    */
show_usr(user)					/*   number		    */
char	user;
{
	printx("User :");
	pdecimal((UWORD) user, 100, TRUE);
}


		/********************************/
		/*		 		*/
		/*    D R I V E S T A T U S     */
		/*		 		*/
		/********************************/

VOID						/* Display status of current*/
drivestatus()					/*   drive		    */
{
	LONG	space;

	print("        ");
	show_drive();
	printx("Drive Characteristics");	/* 128 byte records	    */
	space = (LONG) (dpba->mxa + 1) * kpb;
	p_unl(space * 8);
	printx(record_msg);
	printx(" Capacity");
	p_unl(space);				/* = Kbytes		    */
	printx("Kilobyte Drive  Capacity");
	p_unl((LONG) dpba->dmx + 1);		/* Directory slots	    */
	printx("32 Byte");
	printx(entries);
	p_unl((LONG) dpba->cks * 4);		/* Slots checked to find if */
	printx("Checked");			/*   disk has been changed  */
	printx(entries);
	p_unl(((LONG) dpba->exm + 1) * 128);	/* Records allocated / slot */
	printx(record_msg);
	printx("s / Directory Entry");
	p_unl((LONG) 1 << dpba->bls);		/* Records / block	    */
	printx(record_msg);
	printx("s / Block");
	p_unl((LONG) dpba->spt);		/* Records / track	    */
	printx(record_msg);
	printx("s / Track");
	p_unl((LONG) dpba->ofs);		/* Reserved tracks	    */
	printx("Reserved  Tracks");
	crlf();
}


		/********************************/
		/*		 		*/
		/*     U S E R S T A T U S      */
		/*		 		*/
		/********************************/

VOID						/* Display active user no   */
userstatus()					/*   and user no's which    */
{						/*   own files on current   */
	register UWORD	i;			/*   drive		    */
	BOOLEAN		user[16];

	crlf();					/* Show current user & drive*/
	show_drive();
	printx("Active ");
	show_usr(user_code);
	crlf();
	show_drive();
	printx("Active Files:");		/* Find out who owns files  */
	for (i = 0 ; i < sizeof user / sizeof (BOOLEAN);
		user[i++] = FALSE);		/* (Assume nobody at start) */
	_setdma(buff);
	dcnt = _srch_1st("?");			/* Find first FCB on disk   */

	while ( dcnt != 255 )			/* While more FCB's	    */
	{					/* This expression gets user*/
						/*   no of current file from*/
						/*   directory buffer	    */
		if ((i = ((struct fcbtab *) &buff[(dcnt & 3) * 32])->drive &
		     0xff) != DELETED)		/* Not deleted entry	    */
			user[i & 0x0f] = TRUE;	/* User has at least 1 file */
		dcnt = _srch_next();		/* Find next FCB	    */
	}
						/* Print users with files   */
	for (i = 0; i < sizeof user / sizeof (BOOLEAN); i++)
		if (user[i])
			pdecimal(i, 100, TRUE);
	crlf();
}


		/********************************/
		/*		 		*/
		/*     D I S K S T A T U S      */
		/*		 		*/
		/********************************/

VOID						/* Display status of logged */
diskstatus()					/*   in disk drives	    */
{
	int		d;
	register UWORD	login;			/* *UNSIGNED* login vector  */
						/*    (ensure logical shift)*/

	login = _ret_login();			/* Which disks logged in?   */
	d = 0;

	do					/* While more logged drives */
	{
		if (login & 1)			/* Bit zero shows this	    */
		{				/*   drive is logged	    */
			select_disk(d);
			drivestatus();		/* Tell user about it	    */
		}
		d++ ;				/* Try next drive	    */
	} while (login >>= 1);
}


		/********************************/
		/*		 		*/
		/*         P R N A M E          */
		/*		 		*/
		/********************************/

VOID
prname(a)					/* Print the device name at */
register char	*a;				/*   a.  ':' terminates name*/
{
	do
		_conout(*a);
	while (*a++ != ':');
}


		/********************************/
		/*		 		*/
		/*      D E V S T A T U S       */
		/*		 		*/
		/********************************/

VOID						/* Print logical - physical */
devstatus()					/*   device mapping (from   */
{						/*   iobyte)		    */
	register UWORD	iobyte;			/* Iobyte needs unsigned var*/
	register int	j, k;

	iobyte = _get_iob();
	j = 0;					/* j indexes phys dev group */

	for (k = 0; k < 4; k++)			/* Iobyte maps four logical */
	{					/*   devices (2 bits each)  */
		prname(&devl[k * 4]);		/* Display logical dev name */
		printx(" is ");			/* Each maps to one of four */
						/*   physical devices	    */
		prname(&devr[((iobyte & 3) * 4) + j]);
		j += 16;			/* Index next phys group    */
		iobyte >>= 2;			/*   & next logical device  */
		crlf();
	}
}


		/********************************/
		/*		 		*/
		/*         V A L U E S          */
		/*		 		*/
		/********************************/

VOID						/* Tell the user what STAT  */
values()					/*   can do		    */
{
	register int j, k;

	printx("STAT 2.2");
	crlf();
	print("File Status   : ");
	printx(filename);
	printx(" [SIZE]");
	print("Read Only Disk: d:=RO");
	print("Set Attribute : "); 
	printx(filename);
	printx(sattrib);
	print("Disk Status   : DSK: d:DSK:");
	print("User Status   : USR: d:USR:");
	print("Iobyte Value  : DEV:");
	print("Iobyte Assign :");

	for (j = 0; j < 4; j++)			/* Print four lines, each   */
	{					/*   showing one logical    */
		crlf();				/*   device and four phys-  */
		prname(&devl[j * 4]);		/*   ical devices	    */
		printx(" =");
		for (k = 0; k <= 12; k += 4)
		{
			_conout(' ');
			prname(&devr[(j * 16) + k]);
		}
	}
	crlf();
}


		/********************************/
		/*		 		*/
		/*       P R C O U N T          */
		/*		 		*/
		/********************************/

VOID						/* Print the amount of space*/
prcount()					/*   remaining on the	    */
{						/*   current disk	    */

	LONG	free;				/* The no of free sectors   */

	free = 0;
	if (HAS_GET_DFS(ver))			/* BDOS can return free     */
	{					/*   space into curent DMA  */
		_setdma(&free);			/*   buffer		    */
		_get_dfs(cdisk);		/* Covert from record count */
		free >>= SPKSHF;		/*   to kbytes (8 secs/k)   */
	}
	else					/* Not BDOS 3: we must	    */
	{					/*   get the info from DPB  */
		free = count();
	}
	p_long(free);
	_conout('k');
}


		/********************************/
		/*		 		*/
		/*        P R A L L O C         */
		/*		 		*/
		/********************************/

VOID						/* Print allocation for the */
pralloc()					/*   current disk	    */
{
	crlf();
	show_drive();				/* Is current disk read-only*/
	printx(((rodisk >> cdisk) & 1) ? "RO" : "RW");
	printx(", Free Space: ");
	prcount();
}


		/********************************/
		/*		 		*/
		/*       P R S T A T U S        */
		/*		 		*/
		/********************************/

VOID						/* Print the status of the  */
prstatus()					/*   disk system	    */
{
	register UWORD	login;
	register int	d;

	login = _ret_login();			/* Login vector set	    */
	d = 0;					/* Start on drive A	    */

	while (login)				/* While more drives	    */
	{
		if (login & 1)			/* This disk is logged	    */
		{ 
			select_disk(d);		/* Find out about it	    */
			pralloc();		/* Tell the user about it   */
			login -= 1;		/* clear the bit            */
		}
		login >>= 1; 			/* Next disk to login lsb   */
		d++;
	}
	crlf();
}


		/********************************/
		/*		 		*/
		/*          D O T S             */
		/*		 		*/
		/********************************/

VOID						/* Output a line of i dots  */
dots(i)
register int i;
{
	crlf();
	while (i--)
		_conout('.');
}


		/********************************/
		/*		 		*/
		/*       P R I N T F N          */
		/*		 		*/
		/********************************/

VOID						/* Print current file name  */
printfn(a)					/*   (pointed to by a)	    */
struct fcbtab	*a;
{
	register int	k;

	show_dv();				/* Drive preceeds file name */
	for (k = 0; k < FNAM; k++)		/* Print name in form	    */
	{					/*   "NNNNNNNN.EEE"	    */
		if (k == NSIZE)
			_conout('.');
		_conout(a->fname[k] & 0x7f);
	}
}

/****************************************************************************/
/*									    */
/*	               F I L E   H A N D L I N G                            */
/*	               -------------------------                            */
/*									    */
/****************************************************************************/


		/********************************/
		/*		 		*/
		/*      A T T R I B U T E       */
		/*		 		*/
		/********************************/

						/* Return TRUE if the file  */
						/*   described by fcbsa has */
						/*   attribute a set	    */
						/* NOTE: this is a macro    */

#define attribute(a) (fcbsa->a & 0x80)


		/********************************/
		/*		 		*/
		/*       A L L O C A T E        */
		/*		 		*/
		/********************************/

BOOLEAN						/* Make an allocation vector*/
allocate(block_ptr)				/*   and check for duplicate*/
struct	block_no *block_ptr;			/*   blocks in directory    */
{						/* Return TRUE if allocation*/
						/*   consitent (no duplic-  */
						/*   ates), FALSE otherwise */
						/* Builds an allocation map,*/
						/*   one bit per disk block */
						/*   in memory starting at  */
						/*   alloc, setting bits as */
						/*   it finds blocks alloc- */
						/*   ated		    */
	UWORD	block, vbyte, amask;

	if (word_blks)				/* Block addresses are 16   */
	{					/*   bits long		    */
#ifdef HILO					/* Processor is Z8000, 68K..*/
		block = ((UWORD) block_ptr->second << 8) +
			 (UWORD) block_ptr->first & 0xff;
#else						/* It's 8080, 8086, PDP11...*/
		block = *(UWORD *) block_ptr;
#endif
	}
	else					/* 8-bit block addresses    */
		block = block_ptr->first;
	vbyte = block / 8;			/* Index into alloc vector  */
	amask = 1 << (block % 8);		/* Bit in vector byte	    */
	if (amask & alloc[vbyte])		/* Block already allocated? */
	{					/* Yes			    */
		if (error_free)			/* Disk previously innocent?*/
		{				/* Yes: show its guilt	    */
			error_free = FALSE;
			print("Bad Directory on ");
			show_dv();
			print("Space Allocation Conflict:");
		}
		crlf();				/* Tell user which file is  */
		show_usr(bfcba->drive);		/*   corrupt (drive field   */
		blanks(8);			/*   in FCB holds user #)   */
		printfn(bfcba);
		return (FALSE);
	}
	alloc[vbyte] |= amask; 			/* Block not previously used*/
	return (TRUE);				/*   mark it allocated	    */
}


		/********************************/
		/*		 		*/
		/*      N A M E _ D I F F       */
		/*		 		*/
		/********************************/
int
name_diff(a ,b)					/* Check for matching file  */
register struct fcbtab *a;			/*   names.  Returns -1 if  */
register struct fcbhalf *b;			/*   name of file a lexico- */
{						/*   graphically before that*/
	register int	i, c_a, c_b;		/*   of b, 0 (FALSE) if     */
						/*   same, 1 if after	    */
						/* Wildcards ('?') in name  */
	for (i = 0 ; i < FNAM; i++)		/*   a match any character  */
	{					/*   in name b		    */
		if ((c_a = a->fname[i] & 0x7f) == '?')
			continue;
		if (c_a < (c_b = b->fname[i] & 0x7f)) 
			return (-1);
		if (c_a > c_b)
			return (1);
	}
	return (0);				/* Names NOT different (ie  */
}						/*   they ARE the same)	    */


		/********************************/
		/*		 		*/
		/*   I N D _ N A M E _ D I F F  */
		/*		 		*/
		/********************************/

int 						/* Return ordering info for */
ind_name_diff(p_a, p_b)				/*   for two FCB's (see	    */
struct fcbhalf	**p_a, **p_b;			/*   name_diff above) given */
{						/*   the addresses of two   */
						/*   pointers to FCB's	    */
	return (name_diff((struct fcbtab *) *p_a, *p_b));
}						/* Note: double indirection */
						/* Used by qsort function   */


		/********************************/
		/*		 		*/
		/*     C O U N T _ B L K S      */
		/*		 		*/
		/********************************/

VOID						/* Either check extent at   */
count_blks(allo)				/*   bfcba for consistent   */
BOOLEAN	allo;					/*   allocation (allo TRUE) */
{						/* Or add length of extent  */
	register int	i, mb;			/*   in Kbytes to *fcbma    */

	i = sizeof bfcba->resvd;		/* Start at end of block    */
	while ((i -= (word_blks) ? 2 : 1) >= 0)	/*   pointers and work	    */
	{					/*   towards beginning	    */
		mb = bfcba->resvd[i];		/* Is the current single    */
		if (word_blks)			/*   or double length	    */
			mb |= bfcba->resvd[i + 1];/*  pointer zero?	    */
		if (mb != 0)
		{				/* No: block is allocted    */
			if (allo) 		/* See if consistent	    */
			{
				if (! allocate((struct block_no *)
				     (&bfcba->resvd[i])))
					return;	/*   (give up if not)	    */
			}
			else			/*   or tot up size in	    */
				fcbsa->kcnt += kpb;/*  Kbytes		    */
		}
	}
}


		/********************************/
		/*		 		*/
		/*     C H E C K _ U S E R      */
		/*		 		*/
		/********************************/

VOID						/* Find next file matching  */
check_user()					/*   current name and user  */
{						/* Check all intervening    */
						/*   directory entries for  */
						/*   consistent allocation  */
	while (dcnt != 255)			/* Until directory end	    */
	{					/* Get pointer to this FCB  */
						/*   in default DMA buffer  */
		bfcba = (struct fcbtab *) &buff[dcnt * 32];
						/* Not deleted file or XFCB?*/
						/*   (beware sign extension)*/
		if ((UWORD) (bfcba->drive & 0xff) < 0x20)
		{
			count_blks(TRUE);	/* Check allocation legal   */
			if ((! name_diff(fcb, bfcba)) /* Name & user match? */
			   && ((bfcba->drive & 0x0f) == user_code))
				return;		/* Yes: return		    */
		}
		dcnt = _srch_next();		/* Try next directory entry */
	}
}


		/********************************/
		/*		 		*/
		/*     S E T F S T A T U S      */
		/*		 		*/
		/********************************/

BOOLEAN						/* Parse file attribute	    */
setfstatus()					/*   assignment.  Return    */
{						/*   TRUE if valid assign   */
						/*   found.		    */
	if (! parse_next()) 			/* No more tokens?	    */
		return (FALSE);
	if (token[0] == '=')  			/* Skip optional '='	    */
		scan();
						/* STAT filename SIZE ?	    */
	if ((scase1 = match(attribl, A_SIZE)) == OPT_SIZE) 
	{					/* Yes: not an attribute    */
		sizeset = TRUE;			/*   assignment		    */
		return (FALSE);
	}
	if (scase1 != 0)  			/* If valid attribute does  */
	{					/*   another follow?	    */
		if (parse_next())  		/* If so, is it reasonable? */
		{				/* RO RO, SYS DIR etc are   */
						/*   rejected		    */
			if (((scase2 = match(attribl, A_SIZE)) != 0) 
			   && (abs(scase2 - scase1) > 1)) 
				return (TRUE);	/* Two good attributes	    */
		}
		else
			return (TRUE);		/* One good attribute	    */
	}
	print(invalid);				/* Something wrong.  Print  */
	print(use);				/*   the bad news	    */
	printx(filename);
	printx(" [SIZE] ");
	printx(sattrib);
	_exit(1);				/* User screwed up: abort   */
}


		/********************************/
		/*		 		*/
		/*     S E T _ S A T T R I B    */
		/*		 		*/
		/********************************/

VOID						/* Set/reset selected att-  */
set_sattrib(scase)				/*   ributes of current file*/
int	scase;
{
	switch(scase)
	{
	  case OPT_RO:				/* Read-only		    */
		fcbsa->ROFILE |= 0x80;
		printx(readonly);
		break;
		
	  case OPT_RW:				/* Read-write		    */
		fcbsa->ROFILE &= 0x7f;
		printx(readwrite);
		break;

	  case OPT_SYS:				/* "System" (does not appear*/
		fcbsa->SYSFILE |= 0x80;		/*   in directory listing   */
		printx("System (Sys)");
		break;

	  case OPT_DIR:				/* "Directory" (appears in  */
		fcbsa->SYSFILE &= 0x7f;		/*   directory listing	    */
		printx("Directory (Dir)");
		break;
	   
	  default:
		print(invalid);
	}
}


		/********************************/
		/*		 		*/
		/*     C O M P A R E _ F C B	*/
		/*		 		*/
		/********************************/

BOOLEAN						/* Check whether current FCB*/
compare_fcb()					/*   refers to a file we've */
{						/*   already encountered    */
	register UWORD	i;			/* Return index of matching */
						/*   entry or free slot	    */
	fcbsa = fcbs;
	for (i = 0; i < fcbn; fcbsa++, i++)	/* Points fcbsa at the	    */
	{					/*   matching name, or at   */
		if (! name_diff(fcbsa, bfcba))  /*   next empty slot	    */
			break;
	}

	return (i);
}


		/********************************/
		/*		 		*/
		/*      C O P Y _ F C B         */
		/*		 		*/
		/********************************/

VOID						/* Add the current directory*/
copy_fcb()					/*   entry to the list of   */
{						/*   known files (used when */
						/*   entry has name not	    */
						/*   previously encountered)*/

	fcbn++;					/* Increment count	    */
	if (fcbn > fcbmax)			/* Too many files?	    */
	{ 
		print("Too Many Files");
		_exit(1);			/* Fatal error		    */
	}
	finx[fcbn - 1] = fcbsa;			/* Save index for later sort*/
	*fcbsa = *((struct fcbhalf *) bfcba);	/* Copy FCB.  Clear extent, */
	fcbsa->extent = fcbsa->kcnt = 		/*   byte and record count  */
		fcbsa->rcnt = 0;
}


		/********************************/
		/*		 		*/
		/*   A D D _ F C B _ B L K S    */
		/*		 		*/
		/********************************/

VOID						/* Update the statistics of */
add_fcb_blks()					/*   current file with the  */
{						/*   information from the   */
	register int	kb;			/*   current FCB	    */

	if (bfcba->drive <  0x10)		/* Drive field holds user no*/
	{     
		nfcbs++;			/* Increment fcb count	    */
		for (kb = 0; kb < FNAM; kb++)	/* Turn on any attributes   */ 
		{				/*   that are set in case   */
						/*   missing in previous    */
						/*   extents		    */
			if (bfcba->fname[kb] & 0x80)
				fcbsa->fname[kb] |= 0x80;
		}
		if (bfcba->ARCHIV & 0x80)	/* Turn of archiving if any */
		fcbsa->ARCHIV &= 0x7f;		/*   extent not archived    */
		fcbsa->extent++;		/* Prepare for next extent  */
						/* Tot up logical file size */
						/*   (beware sign extension)*/
		fcbsa->rcnt += (bfcba->reccnt & 0xff) +	
			    (bfcba->extent & dpba->exm) * 128;
		count_blks(FALSE);		/*    & physcal blocks used */
	} 
	else if (bfcba->drive < 0x20)		/* This is an extended FCB  */
	{
		fcbsa->s1 |= 0x80;		/* Set XFCB exists flag	    */
	}
						/* If bfcba->drive >= 0x20, */
						/*    directory entry free  */
}


		/********************************/
		/*		 		*/
		/*        D I S P L A Y         */
		/*		 		*/
		/********************************/

VOID				  		/* Display file details	    */
display()					/*   (STAT afn [SIZE])	    */
{
	register int	add, sizecols;		/* Layout variables	    */
	UWORD		kblks;			/* Total number of 1k blks  */
	UWORD		tall;			/* Total allocation	    */
	BOOLEAN		wide, xfcbfound;
	register int	l;

	tall = 0;
	kblks = 0;

	add = sizecols = 0; 			/* Calculate layout for     */
	if ((wide = (columns() > 48))) 		/*   displayed data	    */
		add = 7;
	if (sizeset)  				/* Printing physical size?  */
		add += (sizecols = 10);
	print(drivename);			/* Show the drive name	    */
	show_drive();
	blanks(17 + add);
	show_usr(user_code);			/*   and user code	    */
	if (sizeset) 				/* Print appropriate header */
		print("     Size "); 		/*   according to data	    */
	else					/*   requested and screen   */
		crlf();				/*   width		    */
	printx(" Recs  Bytes FCBs Attrib");
	if (wide)  
		printx("utes   ");
	printx("   Name");

	for (l = 0; l < fcbn; l++)		/* For each matched file    */
	{
						/*Move FCB to full-size FCB*/
		fcbsa = finx[l];
		*((struct fcbhalf *) fcb) = *fcbsa;
		crlf();

		fcb->drive = 0;
		if (sizeset)			/* Need to print size?	    */
		{ 
			_filsiz(fcb);	 	/* Have BDOS calculate	    */
						/*   virtual file size	    */
#ifdef HILO
			p_long((LONG) fcb->record & 0xffffff);
#else						/* On LOHI machine, shift   */
			p_long((LONG) fcb->record >> 8);/* result to low  */
#endif						/*   end of long word	    */
			_conout(' ');
		}
						/* Following expression gets*/
						/*   index in size table by */
						/*   finding index of curr- */
						/*   ent FCB in name table  */
		pdecimal(fcbsa->rcnt, 10000, TRUE);
						/* Display physical record  */
		_conout(' ');			/*   count		    */
		
		kblks += (fcbsa->rcnt + 7) / 8;	/* Tot up total Kbytes,	    */
						/*   rounding fractions up  */
						/* Print size in Kbytes	    */
		pdecimal((fcbsa->rcnt + 7) / 8, 10000, TRUE);	
		tall += fcbsa->kcnt;
		printx("k ");

		xfcbfound = attribute(HAS_XFCB);/* Does this file have XFCB?*/
		fcbsa->s1 &= 0x7f;		/* Now we know, clear flag  */
						/* === Why? ===		    */
						/* Print no of FCB's	    */
		pdecimal((UWORD) fcbsa->extent, 1000, TRUE);
						/* Display attributes	    */
		printx((attribute(SYSFILE)) ? " Sys " : " Dir ");
		printx((attribute(ROFILE)) ? "RO " : "RW ");

		if (wide)			/* Enough room for more?    */
		{				/* Yes: show more attributes*/
			_conout((xfcbfound) ? 'X' : ' ');
			_conout((attribute(ARCHIV)) ? 'A' : ' ');
			_conout((attribute(ATTRB1)) ? '1' : ' ');
			_conout((attribute(ATTRB2)) ? '2' : ' ');
			_conout((attribute(ATTRB3)) ? '3' : ' ');
			printx((attribute(ATTRB4)) ? "4 " : "  ");
		}
		printfn(fcbsa);			/* At last, the filename!   */
	}
						/* All file info printed:   */
						/*   now do totals	    */
	dots(39 + add);				/* Line up columns neatly   */
	print("Total:");
	blanks(sizecols);
	pdecimal(tall, 10000, TRUE);		/* Total kbytes		    */
	_conout('k');
	pdecimal(nfcbs, 10000, TRUE);		/* Total number of FCB's    */
	printx(" (");				/*   of files		    */
	pdecimal(fcbn, 1000, TRUE);
	printx((fcbn == 1) ? " file" : " files");
	if (wide)				/* Room for more?	    */
	{
		printx((fcbn == 1) ? ", " : ",");
		pdecimal(kblks, 10000, TRUE);	/* Print kbytes used	    */
		printx("-1k blocks");
	}
	_conout(')');
}


		/********************************/
		/*		 		*/
		/*        S E T F A T T         */
		/*		 		*/
		/********************************/

VOID						/* Set the attribtes of all */
setfatt()					/*   matched files to those */
{						/*   requested		    */
	register int	l;

	for (l = 0; l < fcbn; l++)		/* For each matched file    */
	{
		crlf();
		printfn(fcbsa = finx[l]);	/* Print its name	    */
		printx(set_to);			/*   and what its attributes*/
		set_sattrib(scase1);		/*   will be		    */
		if (scase2 != 0)
		{
			printx(", ");
			set_sattrib(scase2);
		}
		fcbsa->drive = 0;		/* Clear user no. (would be */
						/*   interpreted as drive)  */
		_set_att(fcbsa);		/* Go fix attributes	    */
	}
}


		/********************************/
		/*		 		*/
		/*       G E T F I L E          */
		/*		 		*/
		/********************************/

VOID						/* Process request involving*/
getfile()					/*   (possibly ambiguous)   */
{						/*   filename.  The CCP has */
						/*   parsed name and put it */
						/*   in basepage FCB for us */

	if ((set_attribute = setfstatus())  	/* Find out what to do from */
	   && (scase1 == 0))			/*   command line.  Give up */
		return;				/*   on error		    */

	alloc = all_map();			/* Allocate disk map space  */
	fcbmax = all_fcb();			/*   and fcb space	    */

	fcbn = fcb->drive = 0;			/* Search for first file on */
	fcb->extent = fcb->s2 = '?';		/*   current drive 	    */
	dcnt = _srch_1st("?");			/* Read first directory	    */
						/*   sector from current drv*/
	check_user();				/* Do we have a match?	    */

	while (dcnt != 255)			/* While more directory FCBs*/
	{
						/* Get address of this FCB  */
						/*   in directory buffer    */
		bfcba = (struct fcbtab *) &buff[(dcnt & 0x3) * 32];

		if (compare_fcb() >= fcbn)	/* Is this an extent of a   */
			 			/*   file we already met?   */
			copy_fcb();		/* Yes: add name to list    */

		add_fcb_blks();			/* Adjust file's block count*/
		dcnt = _srch_next();		/* Find next directory entry*/
		check_user();
		test_kbd_esc();			/* Give user chance to abort*/
	}

	if (! error_free)			/* Allocation inconsitent?  */
		_exit(1);			/* Yes: user must clean up! */

	if (fcbn == 0)  			/* Did we find any files?   */
	{
		print("File Not Found");	/* No: tell user	    */
		return;
	}

	if (set_attribute)			/* Are we setting file 	    */
	{					/*   attributes?	    */
		setfatt();			/* Yes			    */
		return;
	}

						/* User must want display of*/
						/*   collected data.  First */
						/*   sort names then display*/

	qsort(finx, fcbn, sizeof (struct fcbhalf *), ind_name_diff);
	display();
	pralloc();
}


		/********************************/
		/*		 		*/
		/*        P R D R I V E         */
		/*		 		*/
		/********************************/

VOID						/* Show current drive and   */
prdrive(a)					/*   status to be assigned  */
char	*a;
{
	print(&drivename[1]);			/* print("Drive "),	    */
	show_dv();				/*   name,		    */
	printx(set_to);
	printx(a);				/*   and status		    */
}


		/********************************/
		/*		 		*/
		/*  S E T D R I V E S T A T U S */
		/*		 		*/
		/********************************/

VOID						/* Set current drive to RO  */
setdrivestatus()				/*   or RW status	    */
{
	switch (match(attribl, A_SIZE))
	{
	  case OPT_RO:
		_wr_protd();
		prdrive(readonly);
		break;

	  case OPT_RW:
		if ((_rs_drive(1 << cdisk)) != 0)
			print("Disk Reset Denied");
		else
			prdrive(readwrite);
		break;

	  default:
		print(invalid);
		print(use);
		printx("d:=RO");
	}
}

/****************************************************************************/
/*									    */
/*	          C O M M A N D   P R O C E S S I N G                       */
/*	          -----------------------------------                       */
/*									    */
/****************************************************************************/



		/********************************/
		/*		 		*/
		/*       P A R S E _ I T        */
		/*		 		*/
		/********************************/

VOID						/* Process command line in  */
parse_it()					/*   one of three ways:	    */
{
	switch (match(devl, L_SIZE)) 
	{
	  case OPT_USR:				/* Request user status	    */
		userstatus();
		break;
	
	  case OPT_DSK:				/* Set drive status	    */
		drivestatus();
		break;

	  default:				/* Set/display file details */
		getfile();
	}
}


		/********************************/
		/*		 		*/
		/*         D E V R E Q          */
		/*		 		*/
		/********************************/

BOOLEAN						/* Process a device request */
devreq()					/* Return true if valid	    */
{
	register UWORD	iomask;
	BOOLEAN 	first;
	register int	j, k;

	first = TRUE;
	FOREVER					/* Process each arg in turn */
	{					/* Is this a device name?   */
		if ((j = match(devl, L_SIZE)) == 0)
		{				/* No: error unless first   */
			if (! first) 		/*   token		    */
				goto error;
			return (FALSE);		/* Not a device request	    */
		}
		first = FALSE;			/* Found first/next item    */

		switch (j)			/* What did we find?	    */
		{
		  case 5:			/* Device status request    */
			devstatus();
			break;

		  case 6:			/* List possible assignment */
			values();
			break;
		
		  case 7:			/* List user status values  */
			userstatus();		/* Exit when done (search   */
			_exit(0);		/*   zaps rest of cmd line) */

		  case 8:			/* Show the disks' status   */
			diskstatus();
			break;

						/****************************/
						/*	B E W A R E !	    */
						/* Many hard coded constants*/
						/*   below.  Unless iobyte  */
						/*   changes size, they're  */
						/*   OK			    */
						/****************************/
		  default:			/* Logical-physical device  */
						/*   assignment		    */
		  				/* Scan table item[j - 1]   */
			k = --j * 16;		/* Index to valid devices   */
			if (! parse_assign()) 	/*   for assignment	    */
				goto error;	/* Not assignment: error    */
			if ((k = match(&devr[k], 4) - 1) < 0) 
				goto error;	/* Not valid phys device    */

			iomask = ~3;		/* Mask has two bits clear  */
			while (j--)		/* Find correct iobyte field*/
			{			/* (shift left, 1's fill)   */
				iomask = (iomask << 2) | 3;
				k <<= 2;	/* k holds required mapping */
			}
						/* Replace designated iobyte*/
						/*   field with value in k  */
			_set_iob((_get_iob() & iomask) | k);

		}				/* end of switch	    */

		if (! parse_next()) 		/* If no more tokens, return*/
			return (TRUE);
	}					/* End of forever	    */

error:						/* Invalid command line	    */
	print(invalid);
	return (TRUE);
}


/****************************************************************************/
/*									    */
/*	           S T A T :    M A I N   F U N C T I O N                   */
/*	           --------------------------------------                   */
/*									    */
/****************************************************************************/


VOID
_main()
{
	ver = _version();			/* Get CP/M version number  */
	fcb = &_base->fcb1;			/* Set up pointers to first */
						/*   basepage FCB	    */
	buff = _base->buff;			/*   and to DMA buffer	    */
	cdisk = _ret_cdisk();			/* Find out current drive   */
	user_code = _gset_ucode(0xff);		/*   and the user	    */

	if (! parse_next())  			/* If command line empty,   */
	{
		prstatus();			/*   print status	    */
		_exit(0);			
	}

	if (token[1] == ':')			/* Not empty: drive name?   */
	{
		select_disk(token[0] - 'A');	/* Yes: select it	    */

		if (! parse_next())  		/* Was that all?	    */
		{
			pralloc();		/* Yes: display allocation  */
			_exit(0);
		}

		if (token[0] == '=')		/* No.  Perhaps attribute   */
		{				/*   assignment?	    */
			scan();			/* Yes: get parameter	    */
			setdrivestatus();	/*   and assign		    */
			_exit(0);
		}

		parse_it();			/* Neither of above.  Choose*/
		_exit(0);			/*   from: drive status	    */
						/*	   user status	    */
	}					/*	   file operation   */

	set_kpb();				/* Get current disk details */
	if (! devreq()) 			/* Try to perform device    */
						/*   request.  If that fails*/
		getfile();			/*   must be file request on*/
						/*   current drive	    */
}
