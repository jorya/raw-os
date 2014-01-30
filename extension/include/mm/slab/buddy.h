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



/* 	
 *	2012-9  Slab Memory Managment
 *          by Hu Zhigang <huzhigang.rawos@gmail.com>
 */


#ifndef KERN_BUDDY_H_
#define KERN_BUDDY_H_

#define BUDDY_SYSTEM_INNER_BLOCK	0xff

struct buddy_system;

/** Buddy system operations to be implemented by each implementation. */
typedef struct {
	/**
	 * Return pointer to left-side or right-side buddy for block passed as
	 * argument.
	 */
	link_t *(* find_buddy)(struct buddy_system *, link_t *);
	/**
	 * Bisect the block passed as argument and return pointer to the new
	 * right-side buddy.
	 */
	link_t *(* bisect)(struct buddy_system *, link_t *);
	/** Coalesce two buddies into a bigger block. */
	link_t *(* coalesce)(struct buddy_system *, link_t *, link_t *);
	/** Set order of block passed as argument. */
	void (*set_order)(struct buddy_system *, link_t *, RAW_U8);
	/** Return order of block passed as argument. */
	RAW_U8 (*get_order)(struct buddy_system *, link_t *);
	/** Mark block as busy. */
	void (*mark_busy)(struct buddy_system *, link_t *);
	/** Mark block as available. */
	void (*mark_available)(struct buddy_system *, link_t *);
	/** Find parent of block that has given order  */
	link_t *(* find_block)(struct buddy_system *, link_t *, RAW_U8);
} buddy_system_operations_t;

typedef struct buddy_system {
	/** Maximal order of block which can be stored by buddy system. */
	RAW_U8 max_order;
	link_t *order;
	buddy_system_operations_t *op;
	/** Pointer to be used by the implementation. */
	void *data;
} buddy_system_t;

extern void buddy_system_create(buddy_system_t *, RAW_U8,
    buddy_system_operations_t *, void *);
extern link_t *buddy_system_alloc(buddy_system_t *, RAW_U8);
extern bool buddy_system_can_alloc(buddy_system_t *, RAW_U8);
extern void buddy_system_free(buddy_system_t *, link_t *);
extern size_t buddy_conf_size(size_t);
extern link_t *buddy_system_alloc_block(buddy_system_t *, link_t *);

#endif

/** @}
 */
