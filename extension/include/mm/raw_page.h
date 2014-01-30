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

#ifndef RAW_PAGE_H
#define RAW_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Page
 */
typedef struct PAGE{

	RAW_U8	mem[2048];
	
} PAGE;


#define PAGE_SIZE       (sizeof(PAGE))


/*
 * Page management queue
 */
typedef struct PAGE_DESCRIPTOR {
	RAW_S8  cont;		/* 1 if multiple pages in succession */
	RAW_S8  use;		/* 1 if page in use */
	RAW_S32 next;
	RAW_S32 prev;
} PAGE_DESCRIPTOR;


#define USE         1	/* In use */
#define FREE        0	/* Free */
#define CONT        1	/* Continuation (multiple blocks in succession) */
#define ONE         0	/* Independent */


/* Number of successive pages */
#define PAGE_NUMBERS(q)	( ( (q)->cont )? ((q)+1)->next: 1 )


/*
 * Page management table
 *	Top of pageque is used for freeque.
 *	freeque is sorted in order from the smallest number of
 *	successive free pages.
 */
typedef struct PAGETBL{

	RAW_S32                            maxpage;	/* Total number of pages */
	RAW_S32                            freepage;	/* Number of free pages */
	PAGE_DESCRIPTOR                    *pageque;	/* Array of management information for
				   												all pages */
	PAGE                               *top_page;	/* Top page address */
	RAW_MUTEX                          mem_lock;
	
} PAGETBL;


RAW_U16 raw_page_init(RAW_VOID *start, RAW_VOID *end);
RAW_VOID *raw_page_allocate(RAW_S32 pages);
RAW_S32 raw_page_free(RAW_VOID *adr);
PAGETBL *system_page_table_get(void);


#ifdef __cplusplus
}
#endif


#endif

