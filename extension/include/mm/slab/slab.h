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



#ifndef KERN_SLAB_H_
#define KERN_SLAB_H_



/** Minimum size to be allocated by malloc */
#define SLAB_MIN_MALLOC_W  4

/** Maximum size to be allocated by malloc */
#define SLAB_MAX_MALLOC_W  22

/** Initial Magazine size (TODO: dynamically growing magazines) */
#define SLAB_MAG_SIZE  4

/** If object size is less, store control structure inside SLAB */
#define SLAB_INSIDE_SIZE  (PAGE_SIZE_SLAB >> 3)

/** Maximum wasted space we allow for cache */
#define SLAB_MAX_BADNESS(cache) \
	(((unsigned int) PAGE_SIZE_SLAB << (cache)->order) >> 2)

/* slab_reclaim constants */

/** Reclaim all possible memory, because we are in memory stress */
#define SLAB_RECLAIM_ALL  0x01

/* cache_create flags */

/** Do not use per-cpu cache */
#define SLAB_CACHE_NOMAGAZINE   0x01
/** Have control structure inside SLAB */
#define SLAB_CACHE_SLINSIDE     0x02
/** We add magazine cache later, if we have this flag */
#define SLAB_CACHE_MAGDEFERRED  (0x04 | SLAB_CACHE_NOMAGAZINE)

typedef struct {
	link_t link;
	size_t busy;  /**< Count of full slots in magazine */
	size_t size;  /**< Number of slots in magazine */
	void *objs[];  /**< Slots in magazine */
} slab_magazine_t;

typedef struct {
	slab_magazine_t *current;
	slab_magazine_t *last;
	SPINLOCK_DECLARE(lock);
} slab_mag_cache_t;

typedef struct {
	const char *name;
	
	link_t link;
	
	/* Configuration */
	/** Size of slab position - align_up(sizeof(obj)) */
	size_t size;
	
	int (*constructor)(void *obj, unsigned int kmflag);
	size_t (*destructor)(void *obj);
	
	/** Flags changing behaviour of cache */
	unsigned int flags;
	
	/* Computed values */
	RAW_U8 order;   /**< Order of frames to be allocated */
	size_t objects;  /**< Number of objects that fit in */
	
	/* Statistics */
	atomic_t allocated_slabs;
	atomic_t allocated_objs;
	atomic_t cached_objs;
	/** How many magazines in magazines list */
	atomic_t magazine_counter;
	
	/* Slabs */
	link_t full_slabs;     /**< List of full slabs */
	link_t partial_slabs;  /**< List of partial slabs */
	SPINLOCK_DECLARE(slablock);
	/* Magazines */
	link_t magazines;  /**< List of full magazines */
	SPINLOCK_DECLARE(maglock);
	
	/* Mutex*/
	RAW_MUTEX mutex_slab_alloc;
	RAW_MUTEX mutex_slab_free;
	
	/** CPU cache */
	slab_mag_cache_t *mag_cache;
} slab_cache_t;

extern slab_cache_t *slab_cache_create(const char *, size_t, size_t,
    int (*)(void *, unsigned int), size_t (*)(void *), unsigned int);
extern void slab_cache_destroy(slab_cache_t *);

extern void *slab_alloc(slab_cache_t *, unsigned int);
extern void slab_free(slab_cache_t *, void *);
extern size_t slab_reclaim(unsigned int);

/* slab subsytem initialization */
extern void slab_cache_init(void);
extern void slab_enable_cpucache(void);

/* kconsole debug */
extern void slab_print_list(void);

/* malloc support */
extern void *raw_malloc_flag(size_t, unsigned int);
extern void *raw_slab_malloc(size_t size);
extern void *raw_slab_realloc(void *, size_t, unsigned int);
extern int raw_slab_free(void *);

#endif

/** @}
 */
