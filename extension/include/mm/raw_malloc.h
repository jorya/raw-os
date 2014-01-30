/*
     raw os - Copyright (C)  Lingjun Chen(jorya_txj).

    This file is part of raw os.

    raw os is free software; you can redistribute it it under the terms of the 
    GNU General Public License as published by the Free Software Foundation; 
    either version 3 of the License, or  (at your option) any later version.

    raw os is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. if not, write email to jorya.txj@gmail.com
                                      ---

    A special exception to the LGPL can be applied should you wish to distribute
    a combined work that includes raw os, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/


/* 	2012-7  Created by jorya_txj
  *	xxxxxx   please added here
  */

#ifndef RAW_MALLOC_H
#define RAW_MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Memory allocation control information
 *	In the address, the &areaque position must be aligned in
 *	8 byte units. (The lower three bits must be 0)
 *	The 'nouse' area is used to adjust the address.
 *	It is not possible to write to 'nouse'.
 */
typedef struct MemoryAllocateControlBlock {

	RAW_U64             align;		/* Area used for 8 bytes alignment */

	/* Area queue connects the various partitioned areas of the
	 * allocated page.
	 * Addresses are arranged in ascending order within each page,
	 * and in no particular order between pages. */
	LIST                areaque;
	/* Free queue connects unused areas within the allocated
	 * page. Arranged in order of free area size, starting with
	 * the smallest. */
	LIST                freeque;

	RAW_U32             pagesz;		/* Page size (number of bytes) */
	RAW_S32             testmode;	/* Test mode */
	RAW_MUTEX           mem_lock;
	/* Memory allocate/release function */
	void *(*getblk)( RAW_S32 nblk);
	RAW_S32  (*relblk)( void *ptr );
	
} MACB;

RAW_VOID  raw_malloc_init(void);
RAW_VOID *raw_malloc(RAW_U32 size);
RAW_VOID  raw_free(void *ptr);
RAW_VOID *raw_calloc(RAW_U32 nmemb, RAW_U32 size);
RAW_VOID *raw_realloc(void *ptr, RAW_U32 size);
RAW_U16 check_malloc_test(void *ptr, int mode);



/*
 * Option setting: minimum fragment size
 *	must be size 'sizeof(LIST) * 2' or more.
 */
extern RAW_U32               _mem_minfragment;
/*
 * Correction to align the &areaque position with the 8 byte boundary
 */
#define AlignMACB(macb)      ( (MACB*)((RAW_U32)macb & ~0x00000007U) )


/*
 * Minimum fragmentation unit
 *	Since memory is allocated in ROUNDSZ units,
 *	the lower three bits of the address must be 0.
 *	These low three bits are used in the flag in the area queue.
 */
#define ROUNDSZ		         0x8U
#define ROUND(sz)	         ( ((RAW_U32)(sz) + (ROUNDSZ-1)) & ~(ROUNDSZ-1) )

/* Minimum fragment size */
#define MIN_FRAGMENT         ( _mem_minfragment )

/*
 * Flag that uses the lower bits of the area queue prev
 */
#define AREA_USE              0x00000001U	/* In use */
#define AREA_TOP              0x00000002U	/* Top of page */
#define AREA_END              0x00000004U	/* End of page */
#define AREA_MASK             0x00000007U

#define setAreaFlag(q, f)     ( (q)->previous = (LIST *)((RAW_U32)(q)->previous |  (RAW_U32)(f)) )
#define clrAreaFlag(q, f)     ( (q)->previous = (LIST *)((RAW_U32)(q)->previous & ~(RAW_U32)(f)) )
#define chkAreaFlag(q, f)     ( ((RAW_U32)(q)->previous & (RAW_U32)(f)) != 0 )

#define Mask(x)               ( (LIST *)((RAW_U32)(x) & ~AREA_MASK) )
#define Assign(x, y)          ( (x) = (LIST *)(((RAW_U32)(x) & AREA_MASK) | (RAW_U32)(y)) )

/*
 * Area size
 */
#define AreaSize(aq)          ( (RAW_S8 *)(aq)->next - (RAW_S8 *)((aq) + 1) )
#define FreeSize(fq)          ( (RAW_S8 *)((fq) - 1)->next - (RAW_S8 *)(fq) )


#ifdef __cplusplus
}
#endif


#endif


