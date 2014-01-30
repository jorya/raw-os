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

#include <raw_api.h>
#include <mm/raw_page.h>


static PAGETBL system_page_table;

RAW_INLINE void *page_to_addr(RAW_S32 pn, PAGETBL *pt )
{
	void* adr =	 (void*)(pt->top_page + (pn - 1));
	return adr;
}

RAW_INLINE  RAW_S32 addr_to_page(void *adr, PAGETBL *pt )
{
	return  (RAW_S32)(((PAGE*)(adr) - pt->top_page) + 1);
		
}



/*
 * Address check
 *	Returns TRUE if address is OK.
 */
RAW_INLINE RAW_BOOLEAN check_valid_addr(void *adr, PAGETBL *pt )
{
	if ( adr >= (void*)pt->top_page && adr < (void*)(pt->top_page + pt->maxpage) ) {
		if ( ((RAW_U32)adr % PAGE_SIZE) == 0 ) {
			return RAW_TRUE;
		}
	}
	return RAW_FALSE;
}

/*
 * Set page queue value
 */
RAW_INLINE PAGE_DESCRIPTOR set_page_attr( RAW_BOOLEAN u, RAW_BOOLEAN  c, RAW_S32 n, RAW_S32 p)
{
	PAGE_DESCRIPTOR	 q;
	q.use  = u;
	q.cont = c;
	q.next = n;
	q.prev = p;
	
	return q;
}


/*
 * Insert page queue
 *	Inserts ent directly prior to que.
 */
RAW_INLINE void insert_free_page( RAW_S32 que, RAW_S32 ent, PAGETBL *pt )
{
	PAGE_DESCRIPTOR  *qp = &pt->pageque[que];
	PAGE_DESCRIPTOR  *ep = &pt->pageque[ent];

	ep->prev = qp->prev;
	ep->next = que;
	pt->pageque[qp->prev].next = ent;
	qp->prev = ent;
}

/*
 * Isolate page queue
 *	Removes ent from queue.
 */
RAW_INLINE void remove_free_page( RAW_S32 ent, PAGETBL *pt )
{
	PAGE_DESCRIPTOR	*ep = &pt->pageque[ent];

	if ( ep->next != ent ) {
		pt->pageque[ep->prev].next = ep->next;
		pt->pageque[ep->next].prev = ep->prev;
	}
}



/*
 * Free page queue search
 *	Searches for a queue with n free pages (or the closest
 *	number of free pages greater than n).
 *	If such a queue cannot be found, returns 0 (i.e., freeque).
 */
RAW_INLINE  RAW_S32 search_free_page( RAW_S32 n, PAGETBL *pt )
{
	PAGE_DESCRIPTOR	   *pageque = pt->pageque;
	RAW_S32 	pn = 0;

	while ((pn = pageque[pn].next) > 0) {
		
		if (PAGE_NUMBERS(&pageque[pn]) >= n) {
			return pn;
		}
	}
	
	return 0;
}

/*
 * Append free page
 *	Registers as free pages n consecutive pages starting
 *	from the pn page.
 */
static  void add_page_to_pool( RAW_S32 pn, RAW_S32 n, PAGETBL *pt )
{
	RAW_S32 	ins;
	PAGE_DESCRIPTOR	 *pq = &pt->pageque[pn];

	/* Queue setting */
	pq->use  = FREE;
	pq->cont = ( n > 1 )? CONT: ONE;
	
	if ( n > 1 ) {
		pq[1] = set_page_attr(FREE, CONT, n, 0);
	}
	
	if ( n > 2 ) {
		pq[n-1] = set_page_attr(FREE, CONT, n, 0);
	}

	/* Search for position where free pages added */
	ins = search_free_page(n, pt);

	/* Register free pages */
	insert_free_page(ins, pn, pt);
}

/*
 * Set queue for using page
 */
RAW_INLINE void set_page_used( RAW_S32 pn, RAW_S32 n, PAGETBL *pt )
{
	PAGE_DESCRIPTOR	 *pq = &pt->pageque[pn];

	/* Queue setting */
	pq->use  = USE;
	pq->cont = ( n > 1 )? CONT: ONE;
	
	if (n > 1) {
		pq[1]   = set_page_attr(USE, CONT, n, 0);
	}
	
	if (n > 2) {
		pq[n-1] = set_page_attr(USE, CONT, n, 0);
	}
	
	pt->pageque[pn].next = pt->pageque[pn].prev = pn;
}

/*
 * Allocate page
 */
static void *get_pages( RAW_S32 nblk,  PAGETBL *pt)
{
	RAW_S32 	pn;
	RAW_S32 	free;

	/* Free page search */
	pn = search_free_page(nblk, pt);
	
	if ( pn == 0 ) {
		return 0;
	}
	
	free = PAGE_NUMBERS(&pt->pageque[pn]);

	/* Remove from the free queue all consecutive free pages
	   starting from the pn page */
	remove_free_page(pn, pt);

	/* Extract required pages only */
	set_page_used(pn, nblk,pt);
	free -= nblk;

	if (free) {
		/* Return remaining pages to the free queue */
		add_page_to_pool(pn + (RAW_U32)nblk, free, pt);
	}

	pt->freepage -= nblk;

	return page_to_addr(pn, pt);
}



/*
 * Page release
 *	Returns the total number of pages released
 */
static  RAW_S32 free_pages(void *adr, PAGETBL *pt )
{
	RAW_S32	 pn;
	PAGE_DESCRIPTOR	 *pq;
	RAW_S32 	nblk, free;

	pn = addr_to_page(adr, pt);
	pq = &pt->pageque[pn];

	if ( pq->use == FREE ) {
		return 0;
	}

	/* Number of pages to be released */
	free = nblk = PAGE_NUMBERS(pq);

	if(pn + free > pt->maxpage) {
		
		RAW_ASSERT(0);

	}

	/* Is the right page free? */
	if ( (pq + (RAW_U32)nblk)->use == FREE ) {

		/* Remove free pages next to the free queue */
		remove_free_page(pn+(RAW_S32)nblk, pt);

		/* Merge free pages with released pages */
		nblk += PAGE_NUMBERS(pq + nblk);
	}

	/* Is the left page free?*/
	if ((pn > 1) && ((pq-1)->use == FREE) ) {

		/* Number of free previous pages  */
		RAW_S32 n = ( (pq - 1)->cont )? (pq - 1)->next: 1;

		/* Remove free pages previous to the free queue */
		remove_free_page(pn - (RAW_S32)n, pt);

		/* Merge free pages and released pages */
		pn -= (RAW_U32)n;
		nblk += n;

		/* Although essentially unnecessary, set to FREE in
		   case of erroneous calls trying to release the
		   same address more than once. */
		pq->use = FREE;
	}

	/* Register release page in free queue */
	add_page_to_pool(pn, nblk, pt);

	pt->freepage += free;

	return free;
}



static RAW_U16 init_page_table(void *top, void *end, PAGETBL *pt)
{
	RAW_S32 	memsz, npage;

	/* Align top with 8 byte unit alignment */
	top = (void*)(((RAW_U32)top + 7) & ~0x00000007U);
	memsz = (RAW_S32)((RAW_U32)end - (RAW_U32)top);

	/* Allocate page management table */
	pt->pageque = (PAGE_DESCRIPTOR*)top;

	/* caculate the init number of pages */
	npage = (RAW_S32)(((RAW_U32)memsz - sizeof(PAGE_DESCRIPTOR)) / (PAGE_SIZE + sizeof(PAGE_DESCRIPTOR)));

	/*Align the first page to  PAGE_SIZE*/
	pt->top_page = (PAGE*)(((RAW_U32)(pt->pageque + npage + 1)
					+ (PAGE_SIZE - 1)) / PAGE_SIZE * PAGE_SIZE);

	/* calculate real number of pages */
	npage = (RAW_S32)(((RAW_U32)end - (RAW_U32)pt->top_page) / (RAW_U32)PAGE_SIZE);
	pt->maxpage  = npage;
	pt->freepage = npage;

	pt->pageque[0].cont = ONE;
	pt->pageque[0].use = FREE;
	pt->pageque[0].next = 0;
	pt->pageque[0].prev = 0;
		
	add_page_to_pool(1, npage, pt);
	return RAW_SUCCESS;

}



/*
************************************************************************************************************************
*                                       get system_page_table
*
* Description: get system_page_table for debug purpose
*                  
*
* Arguments 
*                  - - -----
*                 
*				         
*				         
* Returns   : a point to  system_page_table
*                
*
* Note(s)    :
*
*             
************************************************************************************************************************
*/
PAGETBL *system_page_table_get(void)
{

	return &system_page_table;

}


/*
************************************************************************************************************************
*                                       allocate page memory
*
* Description: allocate page memory
*                  
*
* Arguments  adr is the  address to be free.
*                  - - -----
*                 
*				         
*				         
* Returns    allocated memory is returned.
*                0: Failed. Either under interrupt or argument pages is 0.
*
* Note(s)    :This function can not be called under interrupt.
*
*             
************************************************************************************************************************
*/
RAW_VOID *raw_page_allocate(RAW_S32 pages)
{
	RAW_VOID *addr;

	if (!pages) {
		return 0;
	}

	if (raw_int_nesting) {

		return 0;

	}

	raw_mutex_get(&system_page_table.mem_lock, RAW_WAIT_FOREVER);
	addr = get_pages(pages, &system_page_table);
	raw_mutex_put(&system_page_table.mem_lock);
	
	return addr;

}


/*
************************************************************************************************************************
*                                       Page memory free
*
* Description: Free page memory.
*                  
*
* Arguments  adr is the  address to be free.
*                  - - -----
*                 
*				         
*				         
* Returns   freed number of pages
*               0: Failed.Either under interrupt or adr is invalid.
*
* Note(s)    :This function can not be called under interrupt.
*
*             
************************************************************************************************************************
*/
RAW_S32 raw_page_free(RAW_VOID *adr)
{
	RAW_S32 num_free_page;

	if (raw_int_nesting) {

		return 0;

	}
	
	if (!check_valid_addr(adr, &system_page_table)) {

		RAW_ASSERT(0);
		return 0;
	}
	
	raw_mutex_get(&system_page_table.mem_lock, RAW_WAIT_FOREVER);
	num_free_page = free_pages(adr, &system_page_table);
	raw_mutex_put(&system_page_table.mem_lock);
	
	return num_free_page;
}


/*
************************************************************************************************************************
*                                       Page memory init
*
* Description: Create page memory and init it.Page memory is suitable for big memory allocate, probably for 
*                   K bytes or more to  allocate.
*
* Arguments  start is the start address of page memory.
*                  - - -----
*                  end  is the end adress  of page memory.   
*				         
*				         
* Returns   RAW_SUCCESS :  page memory init success.
*
* Note(s)    :
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_page_init(RAW_VOID *start, RAW_VOID *end)
{
	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;

	}
	
	raw_mutex_create(&system_page_table.mem_lock, (RAW_U8 *)"system_page_table", RAW_MUTEX_INHERIT_POLICY, 0);
	init_page_table(start, end, &system_page_table);

	return RAW_SUCCESS;
}

