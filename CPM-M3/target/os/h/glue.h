/*********************************************************************
 *
 * file: glue.h
 *
 *	This file contains declarations related to the mostly system
 *	independent glue code that sits between the system dependent
 *	code and the operation system.
 *
 * revisions:
 *
 *	2013-05-27 rli: original version.
 *
 *	2013-05-30 rli: rename glue_loadarm to glue_loadcom.
 *
 ********************************************************************/

#ifndef glue_h_Included
#define glue_h_Included

/************************
 *
 *	PROTOTYPES
 *
 ************************/

unsigned int glue_callbios( cpm_bpb_t *parm );
unsigned int glue_enterprogram( cpm_basepage_t *parm );
unsigned short int glue_loadcom( void );

#endif /* ndef glue_h_Included */
