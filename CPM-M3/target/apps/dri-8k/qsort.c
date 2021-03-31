/***
 *
 * revisions:
 *
 *	2010-08-20 rli: prototype for _swap
 *
 ***/

/**************************************************************************
*
*			q s o r t   F u n c t i o n
*			---------------------------
*	Copyright 1982 by Digital Research Inc.  All rights reserved.
*
*	"qsort" sorts the "number" of items, each of length "size", starting
*	at "base", using function "compare" to compare 2 items in the vector.
*
*	Calling sequence:
*		ret = qsort(base,number,size,compar)
*	Where:
*		ret = always 0
*		base -> start of items vector
*		number = number of elements in vector
*		size = number of bytes per item in vector
*		compar -> comparator function, taking ptrs to items,
*			returning WORD:
*			compar(a,b) <  0  if  *a < *b
*			compar(a,b) == 0  if  *a == *b
*			compar(a,b) >  0  if  *a > *b
*
*	"qsort" uses the quicksort algorithm by C.A.R. Hoare.
*	Ref: "Software Tools in Pascal" by Kernighan & Plauger.
*****************************************************************************/

#include "portab.h"

MLOCAL VOID _swap( REG BYTE *a, REG BYTE *b, REG WORD wid);

#define LINEPOS(nn) ((nn)*siz+bas)
#define EXCHANGE(aa,bb) _swap(aa,bb,siz)

WORD	qsort(bas,num,siz,cmp)			/* CLEAR FUNCTION ***********/
	BYTE *bas;
	WORD num;
	WORD siz;
	WORD (*cmp)();
{
	REG WORD i,j;
	REG BYTE *pivline;

	if( num > 1 )
	{
		i = 0;
		j = num-1;
		pivline = LINEPOS(j);		/* choose last line for pvt */
		do{
		    while( i<j && (*cmp)(LINEPOS(i),pivline) <= 0 )
			i++;
		    while( j>i && (*cmp)(LINEPOS(j),pivline) >= 0 )
			j--;
		    if( i<j )			/* out of order pair	    */
			EXCHANGE(LINEPOS(i),LINEPOS(j));
		}while( i<j );
		EXCHANGE(LINEPOS(i),pivline);
		if( i < num-1-i )		/* sort shorter subset 1st  */
		{
			qsort( bas, i, siz, cmp);
			qsort( LINEPOS(i+1), num-1-i, siz, cmp);
		} else
		{
			qsort( LINEPOS(i+1), num-1-i, siz, cmp);
			qsort( bas, i, siz, cmp);
		}
	}
	return(0);
}

MLOCAL
VOID _swap(a,b,wid)
	REG BYTE *a;
	REG BYTE *b;
	REG WORD wid;
{
	REG BYTE tmp;
	if( a != b ) 
	    for( ; wid-- > 0; a++, b++ )
	    {
		tmp = *a;
		*a = *b;
		*b = tmp;
	    }
}
