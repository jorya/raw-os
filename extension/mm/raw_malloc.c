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
#include <mm/raw_malloc.h>
#include <lib_string.h>
#include <mm/raw_page.h>


/*your version printf need to be here. please change the following line*/
#define extension_printf  vc_port_printf


/*
 * Minimum fragment size
 *	A size smaller than 'sizeof(LIST ) * 2' will result in malfunction.
 */
RAW_U32 _mem_minfragment = sizeof(LIST) * 2;

static MACB malloc_macb;


#define chkalloc     chkalloc_test


void *_mem_malloc( RAW_U32 size, MACB *macb );
void *_mem_calloc( RAW_U32 nmemb, RAW_U32 size, MACB *macb );
void *_mem_realloc( void *ptr, RAW_U32 size, MACB *macb );
void  _mem_free( void *ptr, MACB *macb );
void  _mem_malloctest( int mode, MACB *macb );
RAW_U16 _mem_malloccheck( void *ptr, MACB *macb );

/*
 * Checks for errors in memory allocation information. When mode < 0,
 * dumps the usage status. When ptr != NULL, checks to see that
 * memory allocation corresponds properly with ptr allocated blocks.
 * If so, returns True.
 */
static RAW_U16 chkalloc_test( void *ptr, int mode, MACB *macb )
{
	LIST	*aq, *nq;
	RAW_U32	usesz = 0, fresz = 0, sz;
	int	usebk = 0, frebk = 0, npage = 0;
	RAW_U8	newpg, ptr_ok;

	/* Checks each area in turn */
	newpg = 1;
	ptr_ok = ( ptr == 0 )? 1: 0;
	for ( aq = macb->areaque.next; aq != &macb->areaque; aq = aq->next ) {

		if ( newpg && !chkAreaFlag(aq, AREA_TOP) ) {
			goto err_found;
		}

		if ( chkAreaFlag(aq, AREA_END) ) {
			if ( newpg ) {
				goto err_found;
			}
			newpg = 1;
			fresz += sizeof(LIST);
			npage++;
			continue;
		}
		newpg = 0;

		nq = aq->next;
		if ( Mask(aq->next) != nq || nq <= aq || Mask(nq->previous) != aq ) {
			goto err_found;
		}
		sz = (RAW_U32)((RAW_S8 *)nq - (RAW_S8 *)aq);
		if ( sz < sizeof(LIST)*3 ) {
			goto err_found;
		}

		if ( chkAreaFlag(aq, AREA_USE) ) {
			usesz += sz;
			++usebk;
			if ( ptr == (void*)(aq+1) ) {
				ptr_ok = 1;
			}
			if ( mode < -1 ) {
				 extension_printf("malloc ptr: 0x%08x [%d B]",
							aq+1, AreaSize(aq));
			}
		} else {
			fresz += sz;
			++frebk;
		}
	}
	if ( !newpg ) {
		goto err_found;
	}

	if ( !ptr_ok ) {
		extension_printf("MALLOC: illegal ptr: 0x%08x", ptr);
		return 0;
	}

	if ( mode < 0 ) {
		extension_printf("MALLOC: %d pages, used: %d [%d blks] free: %d [%d blks]",
		npage, usesz, usebk, fresz, frebk);
	}

	return 1;

err_found:
	extension_printf("MALLOC: block corrupted at 0x%08x", aq);
	return 0;
}



RAW_U16 check_malloc_test(void *ptr, int mode)
{

	return chkalloc_test(ptr, mode, &malloc_macb);

}




/*
 * Byte size -->  number of pages
 */
RAW_INLINE RAW_U32 toPageCount( RAW_U32 size, MACB *macb )
{
	return (size + (macb->pagesz - 1)) / macb->pagesz;
}

/*
 * Free queue search
 *	Searches for a free space with the same size as 'size' or
 *	the next largest.
 *      If none is found, returns &freeque.
 */
LIST *searchFreeArea( RAW_U32 size, MACB *macb )
{
	LIST	*q = &macb->freeque;

	/*
	 * Areas up to 1/4 of page size are searched starting
	 * from the smallest;
         * others are searched starting from the largest.
	 */
	if ( size > macb->pagesz / 4 ) {
		/* Searches in order of increasing size */
		RAW_U32 fsz = 0;
		while ( (q = q->previous) != &macb->freeque ) {
			fsz = (RAW_U32)FreeSize(q);
			if ( fsz <= size ) {
				return ( fsz < size )? q->next: q;
			}
		}
		return ( fsz >= size )? q->next: q;
	} else {
		/* Searches in order of decreasing size */
		while ( (q = q->next) != &macb->freeque ) {
			if ( (RAW_U32)FreeSize(q) >= size ) {
				break;
			}
		}
		return q;
	}
}

/*
 * Registration in free space free queue
 *	Free queue comprises a two-tier structure: a queue linking
 *	areas of differing size in order of size, and a queue
 *	linking areas that are the same size.
 *
 *     macb->freeque
 *      |
 *	|   +-----------------------+		+-----------------------+
 *	|   | AreaQue		    |		| AreaQue		|
 *	|   +-----------------------+		+-----------------------+
 *	+----> FreeQue size order   |	 +--------> FreeQue same size ----->
 *	|   |  FreeQue same size --------+      |   EmptyQue		|
 *	|   |			    |		|			|
 *	|   |			    |		|			|
 *	|   +-----------------------+		+-----------------------+
 *	|   | AreaQue		    |		| AreaQue		|
 *	|   +-----------------------+		+-----------------------+
 */
void appendFreeArea( LIST *aq, MACB *macb )
{
	LIST	*fq;
	RAW_U32	size = (RAW_U32)AreaSize(aq);

	/* Search registration position */
	/*  Searches for a free space with the same size as 'size' or
	 *  the next largest.
	 *  If none is found, returns &freeque.
	 */
	fq = searchFreeArea(size, macb);

	/* Registration */
	clrAreaFlag(aq, AREA_USE);
	if ( fq != &macb->freeque && (RAW_U32)FreeSize(fq) == size ) {
		list_insert(fq + 1, aq + 1);
	} else {
		list_insert(fq, aq + 1);
	}
	list_init(aq + 2);
}

/*
 * Delete from free queue
 */
static void removeFreeQue( LIST  *fq )
{
	if ( !is_list_empty(fq + 1) ) {
		LIST  *nq = (fq + 1)->next;

		list_delete(fq + 1);
		list_insert(nq,nq + 1);
		list_delete(nq);
		list_insert(fq,nq);
	}

	list_delete(fq);
}

/*
 * Area registration
 *	Insert 'ent' directly after 'que'
 */
static void insertAreaQue( LIST  *que, LIST  *ent )
{
	ent->previous = que;
	ent->next = que->next;
	Assign(que->next->previous, ent);
	que->next = ent;
}

/*
 * Delete area
 */
static void removeAreaQue( LIST  *aq )
{
	Mask(aq->previous)->next = aq->next;
	Assign(aq->next->previous, Mask(aq->previous));
}

/*
 * Allocate new page capable of allocating a contiguous area at least
 * as big as 'size' byte
 */
RAW_INLINE LIST *newPage( RAW_U32 size, MACB *macb )
{
	LIST 	*top, *end;
	RAW_U32	nblk;

	if ( macb->pagesz == 0 ) {
		return 0;
	}

	/* Allocate page */
	nblk = toPageCount(size + sizeof(LIST )*2, macb);
	top = (LIST *)(*macb->getblk)(nblk);
	
	if ( top == 0 ) {
		return 0;
	}

	/* Register in area queue */
	end = (LIST *)((RAW_S8 *)top + nblk * macb->pagesz) - 1;
	insertAreaQue(&macb->areaque, end);
	insertAreaQue(&macb->areaque, top);
	setAreaFlag(top, AREA_TOP);
	setAreaFlag(end, AREA_END);

	return top;
}

/*
 * Fragment and allocate
 */
static void *allocate( LIST  *aq, RAW_U32 size, MACB *_macb )
{
	LIST 	*q;
	
	/* Any fragments smaller than the minimum fragment size
	   will also be allocated together */
	if ( (RAW_U32)AreaSize(aq) - size >= MIN_FRAGMENT + sizeof(LIST ) ) {

		/* Divide area in half */
		q = (LIST *)((RAW_S8 *)(aq + 1) + size);
		insertAreaQue(aq, q);

		/* Register surplus area in free queue */
		appendFreeArea(q, _macb);
	}
	setAreaFlag(aq, AREA_USE);

	return (void*)(aq + 1);
}



/* ------------------------------------------------------------------------ */

/*
 * Memory allocate
 */
static void *_mem_malloc(RAW_U32 size, MACB *macb)
{
	LIST	*q;
	if ( macb->testmode) {
		chkalloc(0, 0, macb);
	}

	/* If smaller than the minimum fragment size,
	   allocate the minimum fragment size */
	if ( size > 0 && size < MIN_FRAGMENT ) {
		size = MIN_FRAGMENT;
	}

	size = ROUND(size);
	if ( size <= 0 ) {
		return 0;
	}

	/* Search free queue */
	q = searchFreeArea(size, macb);
	if ( q != &macb->freeque ) {
		/* Free space available: first, isolate from free queue */
		removeFreeQue(q);

		q = q - 1;
	} else {
		/* No free space, then allocate new page */
		q = newPage(size, macb);
		if ( q == 0 ) {
			return 0;  /* Insufficient memory */
		}
	}

	/* Allocate memory */
	return allocate(q, size, macb);
}


/*
 * Memory allocate  and clear
 */
static void *_mem_calloc( RAW_U32 nmemb, RAW_U32 size, MACB *macb )
{
	
	RAW_U32	sz = nmemb * size;
	void	*p;

	/* Allocate memory */
	p = _mem_malloc(sz, macb);
	if ( p == 0 ) {
		return 0;
	}

	/* Memory clear */
	return raw_memset(p, 0, sz);
}



/*
 * Memory allocation size change
 */
static void *_mem_realloc( void *ptr, RAW_U32 size, MACB *macb )
{
	LIST 	*aq;
	RAW_U32	oldsz, sz;
	
	if ( macb->testmode > 0 ) {
		if ( !chkalloc(ptr, 0, macb) ) {
			return 0;
		}
	}

	/* If smaller than minimum fragment size,
	   allocate minimum fragment size */
	if ( size > 0 && size < MIN_FRAGMENT ) {
		size = MIN_FRAGMENT;
	}

	size = ROUND(size);

	aq = (LIST *)ptr - 1;

	if ( ptr != 0 ) {
		/* Current allocation size */
		oldsz = (RAW_U32)AreaSize(aq);

		/* Merge if next space is free space */
		if ( !chkAreaFlag(aq->next, AREA_END|AREA_USE) ) {
			removeFreeQue(aq->next + 1);
			removeAreaQue(aq->next);
		}

		sz = (RAW_U32)AreaSize(aq);
	} else {
		sz = oldsz = 0;
	}

	if ( size <= sz ) {
		if ( size > 0 ) {
			/* Fragment current area and allocate */
			allocate(aq, size, macb);
		} else {
			/* Release area */
			_mem_free(ptr, macb);
			ptr = 0;
		}
	} else {
		/* Allocate new area */
		void *newptr = _mem_malloc(size, macb);
		if ( newptr == 0 ) {
			/* Reallocate original area at original size */
			if ( ptr != 0 ) {
				allocate(aq, oldsz, macb);
			}
			return 0;
		}

		if ( ptr != 0 ) {
			
			/* Copy contents */
			raw_memcpy(newptr, ptr, oldsz);

			/* Release old area */
			_mem_free(ptr, macb);
		}
		ptr = newptr;
	}

	return ptr;
}

/*
 * Free memory
 */
static void  _mem_free( void *ptr, MACB *macb )
{
	LIST	*aq;

	if ( ptr == 0 ) {
		return;
	}

	if ( macb->testmode > 0 ) {
		if ( !chkalloc(ptr, 0, macb) ) {
			return;
		}
	}

	aq = (LIST *)ptr - 1;
	clrAreaFlag(aq, AREA_USE);

	if ( !chkAreaFlag(aq->next, AREA_END|AREA_USE) ) {
		/* Merge with just next free area */
		removeFreeQue(aq->next + 1);
		removeAreaQue(aq->next);
	}

	if ( !chkAreaFlag(aq, AREA_TOP) && !chkAreaFlag(aq->previous, AREA_USE) ) {
		/* Merge with just previous free area */
		aq = aq->previous;
		removeFreeQue(aq + 1);
		removeAreaQue(aq->next);
	}

	/* If whole page is empty, then release the page itself */
	if ( chkAreaFlag(aq, AREA_TOP) && chkAreaFlag(aq->next, AREA_END) ) {
		/* Page release */
		removeAreaQue(aq->next);
		removeAreaQue(aq);
		(*macb->relblk)(aq);
	} else {
		/* Register free area in free queue */
		appendFreeArea(aq, macb);
	}
}


RAW_VOID *raw_malloc(RAW_U32 size)
{
	RAW_VOID *addr;
	MACB	*macb =  &malloc_macb;
	
	if (raw_int_nesting) {

		return 0;

	}

	raw_mutex_get(&macb->mem_lock, RAW_WAIT_FOREVER);
	addr = _mem_malloc(size, macb);
	raw_mutex_put(&macb->mem_lock);
	
	return addr;

}


RAW_VOID raw_free(void *ptr)
{
	MACB	*macb =  &malloc_macb;
	
	if (raw_int_nesting) {

		return;

	}

	raw_mutex_get(&macb->mem_lock, RAW_WAIT_FOREVER);
	_mem_free(ptr, macb);
	raw_mutex_put(&macb->mem_lock);

}


RAW_VOID *raw_calloc(RAW_U32 nmemb, RAW_U32 size)
{

	RAW_VOID *addr;
	MACB	*macb =  &malloc_macb;
	
	if (raw_int_nesting) {

		return 0;

	}

	raw_mutex_get(&macb->mem_lock, RAW_WAIT_FOREVER);
	addr = _mem_calloc(nmemb, size, macb);
	raw_mutex_put(&macb->mem_lock);
	
	return addr;


}



RAW_VOID *raw_realloc(void *ptr, RAW_U32 size)
{

	RAW_VOID *addr;
	
	MACB	*macb =  &malloc_macb;
	
	
	if (raw_int_nesting) {

		return 0;

	}

	raw_mutex_get(&macb->mem_lock, RAW_WAIT_FOREVER);
	addr = _mem_realloc(ptr, size, macb);
	raw_mutex_put(&macb->mem_lock);
	
	return addr;

}



/*
************************************************************************************************************************
*                                      Malloc memory init
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

RAW_VOID raw_malloc_init()
{
	 
	MACB	*macb = (MACB*)((RAW_U32)(&malloc_macb) & ~0x00000007U);
	
	if ((RAW_U32)(&malloc_macb) != (RAW_U32)macb) {
		RAW_ASSERT(0);
	}
		
	list_init(&macb->areaque);
	list_init(&macb->freeque);
	
	macb->pagesz = 2048;
	macb->testmode = 0;
	macb->getblk   = raw_page_allocate;
	macb->relblk   = raw_page_free;
	
	raw_mutex_create(&macb->mem_lock, (RAW_U8 *)"malloc_lock", RAW_MUTEX_INHERIT_POLICY, 0);

}

