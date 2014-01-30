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





#ifndef KERN_FRAME_H_
#define KERN_FRAME_H_

#define ONE_FRAME    0
#define TWO_FRAMES   1
#define FOUR_FRAMES  2


#ifdef ARCH_STACK_FRAMES
	#define STACK_FRAMES  ARCH_STACK_FRAMES
#else
	#define STACK_FRAMES  ONE_FRAME
#endif

/** Maximum number of zones in the system. */
#define ZONES_MAX  32

typedef RAW_U32 pfn_t; /*arch depend huzhigang*/

#define FRAME_WIDTH  12  /* 4K */
#define FRAME_SIZE   (1 << FRAME_WIDTH)

typedef RAW_U8 frame_flags_t;

/** Convert the frame address to kernel VA. */
#define FRAME_KA          0x01
/** Do not panic and do not sleep on failure. */
#define FRAME_ATOMIC      0x02
/** Do not start reclaiming when no free memory. */
#define FRAME_NO_RECLAIM  0x04

typedef RAW_U8 zone_flags_t;

/** Available zone (free for allocation) */
#define ZONE_AVAILABLE  0x00
/** Zone is reserved (not available for allocation) */
#define ZONE_RESERVED   0x08
/** Zone is used by firmware (not available for allocation) */
#define ZONE_FIRMWARE   0x10

/** Currently there is no equivalent zone flags
    for frame flags */
#define FRAME_TO_ZONE_FLAGS(frame_flags)  0

typedef struct {
	size_t refcount;      /**< Tracking of shared frames */
	RAW_U8 buddy_order;  /**< Buddy system block order */
	link_t buddy_link;    /**< Link to the next free block inside
                               one order */
	void *parent;         /**< If allocated by slab, this points there */
} frame_t;

typedef struct {
	pfn_t base;                    /**< Frame_no of the first frame
                                        in the frames array */
	size_t count;                  /**< Size of zone */
	size_t free_count;             /**< Number of free frame_t
                                        structures */
	size_t busy_count;             /**< Number of busy frame_t
                                        structures */
	zone_flags_t flags;            /**< Type of the zone */
	
	frame_t *frames;               /**< Array of frame_t structures
                                        in this zone */
	buddy_system_t *buddy_system;  /**< Buddy system for the zone */
} zone_t;

/*
 * The zoneinfo.lock must be locked when accessing zoneinfo structure.
 * Some of the attributes in zone_t structures are 'read-only'
 */

typedef struct {
	IRQ_SPINLOCK_DECLARE(lock);
	size_t count;
	zone_t info[ZONES_MAX];
} zones_t;

extern zones_t zones;

RAW_INLINE uintptr_t PFN2ADDR(pfn_t frame)
{
	return (uintptr_t) (frame << FRAME_WIDTH);
}

RAW_INLINE pfn_t ADDR2PFN(uintptr_t addr)
{
	return (pfn_t) (addr >> FRAME_WIDTH);
}

RAW_INLINE size_t SIZE2FRAMES(size_t size)
{
	if (!size)
		return 0;
	return (size_t) ((size - 1) >> FRAME_WIDTH) + 1;
}

RAW_INLINE size_t FRAMES2SIZE(size_t frames)
{
	return (size_t) (frames << FRAME_WIDTH);
}

RAW_INLINE bool zone_flags_available(zone_flags_t flags)
{
	return ((flags & (ZONE_RESERVED | ZONE_FIRMWARE)) == 0);
}

#define IS_BUDDY_ORDER_OK(index, order) \
    ((~(((sysarg_t) -1) << (order)) & (index)) == 0)
#define IS_BUDDY_LEFT_BLOCK(zone, frame) \
    (((frame_index((zone), (frame)) >> (frame)->buddy_order) & 0x01) == 0)
#define IS_BUDDY_RIGHT_BLOCK(zone, frame) \
    (((frame_index((zone), (frame)) >> (frame)->buddy_order) & 0x01) == 1)
#define IS_BUDDY_LEFT_BLOCK_ABS(zone, frame) \
    (((frame_index_abs((zone), (frame)) >> (frame)->buddy_order) & 0x01) == 0)
#define IS_BUDDY_RIGHT_BLOCK_ABS(zone, frame) \
    (((frame_index_abs((zone), (frame)) >> (frame)->buddy_order) & 0x01) == 1)

#define frame_alloc(order, flags) \
    frame_alloc_generic(order, flags, NULL)

extern void frame_init(RAW_U32 start, RAW_U32 size);

extern void *frame_alloc_generic(RAW_U8, frame_flags_t, size_t *);
extern void frame_free(uintptr_t);
extern void frame_reference_add(pfn_t);

extern size_t find_zone(pfn_t frame, size_t count, size_t hint);
extern size_t zone_create(pfn_t, size_t, pfn_t, zone_flags_t);
extern void *frame_get_parent(pfn_t, size_t);
extern void frame_set_parent(pfn_t, void *, size_t);
extern void frame_mark_unavailable(pfn_t, size_t);
extern size_t zone_conf_size(size_t);
extern bool zone_merge(size_t, size_t);
extern void zone_merge_all(void);
extern RAW_U64 zones_total_size(void);
extern void zones_stats(RAW_U64 *, RAW_U64 *,RAW_U64 *, RAW_U64 *);

/*
 * Console functions
 */
extern void zones_print_list(void);
extern void zone_print_one(size_t);

#endif

/** @}
 */
