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
 *
 *  2012-9  Add the feature of checking whether free freed obj
 *			by Hu Zhigang <huzhigang.rawos@gmail.com>
 */

#include <raw_api.h>
#include <fifo.h>
#include <lib_string.h>
#include <mm/slab/config.h>
#include <mm/slab/misc.h>
#include <mm/slab/spinlock.h>
#include <mm/slab/arch.h>
#include <mm/slab/list.h>
#include <mm/slab/buddy.h>
#include <mm/slab/frame.h>
#include <mm/slab/slab.h>




static link_t slab_cache_list = 
{
&slab_cache_list,
&slab_cache_list
};

extern config_t config;

/** Magazine cache */
static slab_cache_t mag_cache;

/*Free freed obj check*/
#define SUCCESS	0
#define FAILUE	1

#define ON		0
#define OFF		1

RAW_U8 free_check_switch;

/** Cache for cache descriptors */
static slab_cache_t slab_cache_cache;

/** Cache for external slab descriptors
 * This time we want per-cpu cache, so do not make it static
 * - using slab for internal slab structures will not deadlock,
 *   as all slab structures are 'small' - control structures of
 *   their caches do not require further allocation
 */
static slab_cache_t *slab_extern_cache;

/** Caches for malloc */
static slab_cache_t *malloc_caches[SLAB_MAX_MALLOC_W - SLAB_MIN_MALLOC_W + 1];

static const char *malloc_names[] =  {
	"malloc-16",
	"malloc-32",
	"malloc-64",
	"malloc-128",
	"malloc-256",
	"malloc-512",
	"malloc-1K",
	"malloc-2K",
	"malloc-4K",
	"malloc-8K",
	"malloc-16K",
	"malloc-32K",
	"malloc-64K",
	"malloc-128K",
	"malloc-256K",
	"malloc-512K",
	"malloc-1M",
	"malloc-2M",
	"malloc-4M"
};

/** Slab descriptor */
typedef struct {
	slab_cache_t *cache;  /**< Pointer to parent cache. */
	link_t link;          /**< List of full/partial slabs. */
	void *start;          /**< Start address of first available item. */
	size_t available;     /**< Count of available items in this slab. */
	size_t nextavail;     /**< The index of next available item. */
} slab_t;


#define CONFIG_DEBUG 1
#ifdef CONFIG_DEBUG
static unsigned int _slab_initialized = 0;
#endif

/**************************************/
/* Slab allocation functions          */
/**************************************/

/** Allocate frames for slab space and initialize
 *
 */
 static slab_t *slab_space_alloc(slab_cache_t *cache,
    unsigned int flags)
{
	
	
	size_t zone = 0;
	slab_t *slab;
	size_t fsize;
	size_t i;

	void *data = frame_alloc_generic(cache->order, FRAME_KA | flags, &zone);
	if (!data) {
		return NULL;
	}
	

	
	if (!(cache->flags & SLAB_CACHE_SLINSIDE)) {
		slab = slab_alloc(slab_extern_cache, flags);
		
		if (!slab) {
			frame_free((uintptr_t)data);
			return NULL;
		}
	} else {
		fsize = ( PAGE_SIZE_SLAB << cache->order);
		slab = (slab_t *)((char *)data + fsize - sizeof(*slab));
		
	}
	
	/* Fill in slab structures */

	for (i = 0; i < ((size_t) 1 << cache->order); i++)
		frame_set_parent(ADDR2PFN((uintptr_t)data) + i, slab, zone);
	
	slab->start = data;
	slab->available = cache->objects;
	slab->nextavail = 0;
	slab->cache = cache;
	
	for (i = 0; i < cache->objects; i++)
		*((size_t *) ((char *)slab->start + i * cache->size)) = i + 1;
	
	atomic_inc(&cache->allocated_slabs);
	return slab;
}

/** Deallocate space associated with slab
 *
 * @return number of freed frames
 *
 */
 static size_t slab_space_free(slab_cache_t *cache, slab_t *slab)
{
	frame_free((uintptr_t)slab->start);
	if (!(cache->flags & SLAB_CACHE_SLINSIDE))
		slab_free(slab_extern_cache, slab);
	
	atomic_dec(&cache->allocated_slabs);
	
	return (1 << cache->order);
}

/** Map object to slab structure */
 static slab_t *obj2slab(void *obj)
{
	return (slab_t *) frame_get_parent(ADDR2PFN((uintptr_t)(obj)), 0);
}



/*
 *To check whether there is a same obj with paramter obj 
 *in the magazines. If yes, return FAILED.
 */
static int check_free_freed_obj_mag(slab_cache_t *cache, void *obj)
{
	size_t i;
	link_t *cur;
	slab_magazine_t *cmag = NULL;
	slab_magazine_t *mag = NULL;
    slab_magazine_t *lastmag = NULL;

   if(!cache->mag_cache)
	   return 0;

   cmag = cache->mag_cache->current;
   lastmag = cache->mag_cache->last;


   /*check if there is the same obj in the cmag*/
   if (cmag) {
	   for(i = 0; i < cmag->busy; i++){
		   if(obj == cmag->objs[i])
			   return FAILUE;
	   }
   }

   /*check if there is the same obj in the lastmag*/
   if (lastmag) {
	   for(i = 0; i < lastmag->busy; i++){
		   if(obj == lastmag->objs[i])
			   return FAILUE;
	   }
   }

   /*check if there is the same obj in the magazines list*/
   if (!list_empty(&cache->magazines)) {
	   cur = cache->magazines.next;
	   do{
		   mag = list_get_instance(cur, slab_magazine_t, link);
		   for(i = 0; i < mag->busy; i++){
			   if(obj == mag->objs[i])
				   return FAILUE;
		   }
		   cur = cur->next;
	   }while(cur != cache->magazines.next);
   }

   return 0;
}


 /*
 *To check whether there is a same obj with paramter obj 
 *in the slab obj list. If yes, return FAILED.
 */
static int check_free_freed_obj_slablist(slab_cache_t *cache, slab_t *slab, void *obj)
{
   size_t i = 0;
   void *object = NULL;
   size_t nextavail = 0;
   
   if(!slab)
	   return 0;

   nextavail = slab->nextavail;
   
   for(i = 0; i < slab->available; i++){
	   object = (char *)slab->start + nextavail * cache->size;
	   if(obj == object)
		   return FAILUE;
	   nextavail = *((size_t *) object);
   }


   return 0;
}

/******************/
/* Slab functions */
/******************/

/** Return object to slab and call a destructor
 *
 * @param slab If the caller knows directly slab of the object, otherwise NULL
 *
 * @return Number of freed pages
 *
 */
 static size_t slab_obj_destroy(slab_cache_t *cache, void *obj,
    slab_t *slab)
{
	size_t freed = 0;

	if (!slab)
		slab = obj2slab(obj);
	
	ASSERT(slab->cache == cache);
	
	if (cache->destructor)
		freed = cache->destructor(obj);
	
	spinlock_lock(&cache->slablock);
	ASSERT(slab->available < cache->objects);
	
	*((size_t *) obj) = slab->nextavail;
	slab->nextavail = ((char *)obj - (char *)slab->start) / cache->size;
	slab->available++;
	
	/* Move it to correct list */
	if (slab->available == cache->objects) {
		/* Free associated memory */
		list_remove(&slab->link);
		spinlock_unlock(&cache->slablock);
		
		return freed + slab_space_free(cache, slab);
	} else if (slab->available == 1) {
		/* It was in full, move to partial */
		list_remove(&slab->link);
		list_prepend(&slab->link, &cache->partial_slabs);
	}
	
	spinlock_unlock(&cache->slablock);
	return freed;
}

/** Take new object from slab or create new if needed
 *
 * @return Object address or null
 *
 */
 static void *slab_obj_create(slab_cache_t *cache, unsigned int flags)
{
	slab_t *slab;
	void *obj;

	spinlock_lock(&cache->slablock);
	
	if (list_empty(&cache->partial_slabs)) {
		/*
		 * Allow recursion and reclaiming
		 * - this should work, as the slab control structures
		 *   are small and do not need to allocate with anything
		 *   other than frame_alloc when they are allocating,
		 *   that's why we should get recursion at most 1-level deep
		 *
		 */
		spinlock_unlock(&cache->slablock);

		slab = slab_space_alloc(cache, flags);

		if (!slab)
			return NULL;
		
		spinlock_lock(&cache->slablock);
	} else {
		slab = list_get_instance(cache->partial_slabs.next, slab_t,
		    link);
		list_remove(&slab->link);
	}
	
	obj = (char *)slab->start + slab->nextavail * cache->size;
	slab->nextavail = *((size_t *) obj);
	slab->available--;
	
	if (!slab->available)
		list_prepend(&slab->link, &cache->full_slabs);
	else
		list_prepend(&slab->link, &cache->partial_slabs);
	
	spinlock_unlock(&cache->slablock);
	
	if ((cache->constructor) && (cache->constructor(obj, flags))) {
		/* Bad, bad, construction failed */
		slab_obj_destroy(cache, obj, slab);
		return NULL;
	}
	
	return obj;
}

/****************************/
/* CPU-Cache slab functions */
/****************************/

/** Find a full magazine in cache, take it from list and return it
 *
 * @param first If true, return first, else last mag.
 *
 */
 static slab_magazine_t *get_mag_from_cache(slab_cache_t *cache,
    bool first)
{
	slab_magazine_t *mag = NULL;
	link_t *cur;
	
	spinlock_lock(&cache->maglock);
	if (!list_empty(&cache->magazines)) {
		if (first)
			cur = cache->magazines.next;
		else
			cur = cache->magazines.prev;
		
		mag = list_get_instance(cur, slab_magazine_t, link);
		list_remove(&mag->link);
		atomic_dec(&cache->magazine_counter);
	}
	
	spinlock_unlock(&cache->maglock);
	return mag;
}

/** Prepend magazine to magazine list in cache
 *
 */
 static void put_mag_to_cache(slab_cache_t *cache,
    slab_magazine_t *mag)
{
	spinlock_lock(&cache->maglock);
	
	list_prepend(&mag->link, &cache->magazines);/*please refer <adt/list.h> */
	atomic_inc(&cache->magazine_counter);
	
	spinlock_unlock(&cache->maglock);
}

/** Free all objects in magazine and free memory associated with magazine
 *
 * @return Number of freed pages
 *
 */
 static size_t magazine_destroy(slab_cache_t *cache,
    slab_magazine_t *mag)
{
	size_t i;
	size_t frames = 0;
	
	for (i = 0; i < mag->busy; i++) {
		frames += slab_obj_destroy(cache, mag->objs[i], NULL);
		atomic_dec(&cache->cached_objs);
	}
	
	slab_free(&mag_cache, mag);
	
	return frames;
}

/** Find full magazine, set it as current and return it
 *
 */
 static slab_magazine_t *get_full_current_mag(slab_cache_t *cache)
{
	slab_magazine_t *cmag = cache->mag_cache->current;
	slab_magazine_t *lastmag = cache->mag_cache->last;
	slab_magazine_t *newmag;
	
	
	if (cmag) { /* First try local CPU magazines */
		if (cmag->busy)
			return cmag;
		
		if ((lastmag) && (lastmag->busy)) {
			cache->mag_cache->current = lastmag;
			cache->mag_cache->last = cmag;
			return lastmag;
		}
	}
	
	/* Local magazines are empty, import one from magazine list */
    newmag = get_mag_from_cache(cache, 1);
	if (!newmag)
		return NULL;
	
	if (lastmag)
		magazine_destroy(cache, lastmag);
	
	cache->mag_cache->last = cmag;
	cache->mag_cache->current = newmag;
	
	return newmag;
}

/** Try to find object in CPU-cache magazines
 *
 * @return Pointer to object or NULL if not available
 *
 */
 static void *magazine_obj_get(slab_cache_t *cache)
{
	slab_magazine_t *mag;
	void *obj;
	
	spinlock_lock(&cache->mag_cache->lock);
	
	mag = get_full_current_mag(cache);
	if (!mag) {
		spinlock_unlock(&cache->mag_cache->lock);
		return NULL;
	}
	
	obj = mag->objs[--mag->busy];
	spinlock_unlock(&cache->mag_cache->lock);
	
	atomic_dec(&cache->cached_objs);

	return obj;
}

/** Assure that the current magazine is empty, return pointer to it,
 * or NULL if no empty magazine is available and cannot be allocated
 *
 * We have 2 magazines bound to processor.
 * First try the current.
 * If full, try the last.
 * If full, put to magazines list.
 *
 */
 static slab_magazine_t *make_empty_current_mag(slab_cache_t *cache)
{
	slab_magazine_t *cmag = cache->mag_cache->current;
	slab_magazine_t *lastmag = cache->mag_cache->last;
	slab_magazine_t *newmag;
	
	if (cmag) {
		if (cmag->busy < cmag->size)
			return cmag;
		
		if ((lastmag) && (lastmag->busy < lastmag->size)) {
			cache->mag_cache->last = cmag;
			cache->mag_cache->current = lastmag;
			return lastmag;
		}
	}
	
	/* current | last are full | nonexistent, allocate new */
	
	/*
	 * We do not want to sleep just because of caching,
	 * especially we do not want reclaiming to start, as
	 * this would deadlock.
	 *
	 */
	newmag = slab_alloc(&mag_cache,
	    FRAME_ATOMIC | FRAME_NO_RECLAIM);/*allocate a new empty magazine*/
	if (!newmag)
		return NULL;
	
	newmag->size = SLAB_MAG_SIZE;
	newmag->busy = 0;
	
	/* Flush last to magazine list */
	if (lastmag)
		put_mag_to_cache(cache, lastmag);
	
	/* Move current as last, save new as current */
	cache->mag_cache->last = cmag;
	cache->mag_cache->current = newmag;
	
	return newmag;
}



/** Put object into CPU-cache magazine
 *
 * @return 0 on success, -1 on no memory
 *
 */
 static int magazine_obj_put(slab_cache_t *cache, void *obj)
{
	slab_magazine_t *mag;
	
	spinlock_lock(&cache->mag_cache->lock);
	
	mag = make_empty_current_mag(cache);
	if (!mag) {
		spinlock_unlock(&cache->mag_cache->lock);
		return -1;
	}
	
	mag->objs[mag->busy++] = obj;
	
	spinlock_unlock(&cache->mag_cache->lock);
	
	atomic_inc(&cache->cached_objs);
	
	return 0;
}

/************************/
/* Slab cache functions */
/************************/

/** Return number of objects that fit in certain cache size
 *
 */
 static size_t comp_objects(slab_cache_t *cache)
{
	if (cache->flags & SLAB_CACHE_SLINSIDE)
		return (( PAGE_SIZE_SLAB << cache->order)
		    - sizeof(slab_t)) / cache->size;
	else
		return ( PAGE_SIZE_SLAB << cache->order) / cache->size;
}

/** Return wasted space in slab
 *
 */
 static size_t badness(slab_cache_t *cache)
{
	size_t objects = comp_objects(cache);
	size_t ssize =  PAGE_SIZE_SLAB << cache->order;
	
	if (cache->flags & SLAB_CACHE_SLINSIDE)
		ssize -= sizeof(slab_t);
	
	return ssize - objects * cache->size;
}

/** Initialize mag_cache structure in slab cache
 *
 */
 static bool make_magcache(slab_cache_t *cache)
{
#ifdef CONFIG_DEBUG
	ASSERT(_slab_initialized >= 2);
#endif
	cache->mag_cache = raw_malloc_flag(sizeof(slab_mag_cache_t),
	    FRAME_ATOMIC);
	if (!cache->mag_cache)
		return false;

	raw_memset(cache->mag_cache, 0, sizeof(cache->mag_cache));

	return true;
}

/** Initialize allocated memory as a slab cache
 *
 */
 static void _slab_cache_create(slab_cache_t *cache, const char *name,
    size_t size, size_t align, int (*constructor)(void *obj,
    unsigned int kmflag), size_t (*destructor)(void *obj), unsigned int flags)
{
	size_t pages;

	raw_memset(cache, 0, sizeof(*cache));

	raw_mutex_create(&cache->mutex_slab_alloc, (RAW_U8 *)"mutex_slab_alloc", RAW_MUTEX_INHERIT_POLICY, 0);
	raw_mutex_create(&cache->mutex_slab_free, (RAW_U8 *)"mutex_slab_free", RAW_MUTEX_INHERIT_POLICY, 0);

	cache->name = name;
	
	if (align < sizeof(sysarg_t))
		align = sizeof(sysarg_t);
	
	size = ALIGN_UP(size, align);
	
	cache->size = size;
	cache->constructor = constructor;
	cache->destructor = destructor;
	cache->flags = flags;
	
	list_initialize(&cache->full_slabs);
	list_initialize(&cache->partial_slabs);
	list_initialize(&cache->magazines);
	
	spinlock_initialize(&cache->slablock, "slab.cache.slablock");
	spinlock_initialize(&cache->maglock, "slab.cache.maglock");

	if (!(cache->flags & SLAB_CACHE_NOMAGAZINE))
		(void) make_magcache(cache);
	
	/* Compute slab sizes, object counts in slabs etc. */
	if (cache->size < SLAB_INSIDE_SIZE) 
		cache->flags |= SLAB_CACHE_SLINSIDE;
	
	/* Minimum slab order */
	pages = SIZE2FRAMES(cache->size);

	/* We need the 2^order >= pages */
	if (pages == 1)
		cache->order = 0;
	else
		cache->order = fnzb(pages - 1) + 1;
	
	while (badness(cache) > SLAB_MAX_BADNESS(cache))
		cache->order += 1;
	
	cache->objects = comp_objects(cache);

	/* If info fits in, put it inside */
	if (badness(cache) > sizeof(slab_t))
		cache->flags |= SLAB_CACHE_SLINSIDE;

	/* Add cache to cache list */
	
	list_append(&cache->link, &slab_cache_list); 
	
}


slab_cache_t *slab_cache_create(const char *name, size_t size, size_t align,
    int (*constructor)(void *obj, unsigned int kmflag),
    size_t (*destructor)(void *obj), unsigned int flags)
{
	slab_cache_t *cache = (slab_cache_t *)slab_alloc(&slab_cache_cache, 0);
	_slab_cache_create(cache, name, size, align, constructor, destructor,
	    flags);

	return cache;
}

/** Reclaim space occupied by objects that are already free
 *
 * @param flags If contains SLAB_RECLAIM_ALL, do aggressive freeing
 *
 * @return Number of freed pages
 *
 */
 static size_t _slab_reclaim(slab_cache_t *cache, unsigned int flags)
{
	slab_magazine_t *mag;
	size_t frames = 0;
	atomic_count_t magcount;

	if (cache->flags & SLAB_CACHE_NOMAGAZINE)
		return 0; /* Nothing to do */
	
	/*
	 * We count up to original magazine count to avoid
	 * endless loop
	 */
	magcount = atomic_get(&cache->magazine_counter);
	
	while ((magcount--) && (mag = get_mag_from_cache(cache, 0))) {
		frames += magazine_destroy(cache, mag);
		if ((!(flags & SLAB_RECLAIM_ALL)) && (frames))
			break;
	}
	
	if (flags & SLAB_RECLAIM_ALL) {
		/* Free cpu-bound magazines */
		/* Destroy CPU magazines */

		spinlock_lock(&cache->mag_cache->lock);

		mag = cache->mag_cache->current;
		if (mag)
			frames += magazine_destroy(cache, mag);
		cache->mag_cache->current = NULL;
			
		mag = cache->mag_cache->last;
		if (mag)
			frames += magazine_destroy(cache, mag);
		cache->mag_cache->last = NULL;

		spinlock_unlock(&cache->mag_cache->lock);
	}
	
	return frames;
}

/** Check that there are no slabs and remove cache from system
 *
 */
void slab_cache_destroy(slab_cache_t *cache)
{
	/*
	 * First remove cache from link, so that we don't need
	 * to disable interrupts later
	 *
	 */
	
	list_remove(&cache->link);
	
	
	/*
	 * Do not lock anything, we assume the software is correct and
	 * does not touch the cache when it decides to destroy it
	 *
	 */
	
	/* Destroy all magazines */
	_slab_reclaim(cache, SLAB_RECLAIM_ALL);
	
	/* All slabs must be empty */
	if ((!list_empty(&cache->full_slabs)) ||
	    (!list_empty(&cache->partial_slabs)))
		panic("Destroying cache that is not empty.");
	
	if (!(cache->flags & SLAB_CACHE_NOMAGAZINE))
		raw_slab_free(cache->mag_cache);
	
	slab_free(&slab_cache_cache, cache);
}

/** Allocate new object from cache - if no flags given, always returns memory
 *
 */
void *slab_alloc(slab_cache_t *cache, unsigned int flags)
{
	void *result = NULL;

	raw_mutex_get(&cache->mutex_slab_alloc, RAW_WAIT_FOREVER);
	if (!(cache->flags & SLAB_CACHE_NOMAGAZINE))
		result = magazine_obj_get(cache);

	if (!result)
		result = slab_obj_create(cache, flags);
	
	
	if (result)
		atomic_inc(&cache->allocated_objs);

	raw_mutex_put(&cache->mutex_slab_alloc);
	
	return result;
}

/** Return object to cache, use slab if known
 *
 */
 static int _slab_free(slab_cache_t *cache, void *obj, slab_t *slab)
{
	int ret = 0;

	raw_mutex_get(&cache->mutex_slab_free, RAW_WAIT_FOREVER);

	if(free_check_switch == ON){
		ret = check_free_freed_obj_mag(cache, obj);
		if(FAILUE == ret)
			return ret;

		ret = check_free_freed_obj_slablist(cache, slab, obj);
		if(FAILUE== ret)
			return ret;
	}

	if ((cache->flags & SLAB_CACHE_NOMAGAZINE) ||
	    (magazine_obj_put(cache, obj)))
		slab_obj_destroy(cache, obj, slab);
	
	atomic_dec(&cache->allocated_objs);

	raw_mutex_put(&cache->mutex_slab_free);

	return 0;
}

/** Return slab object to cache
 *
 */
void slab_free(slab_cache_t *cache, void *obj)
{
	_slab_free(cache, obj, NULL);
}

/** Go through all caches and reclaim what is possible */
size_t slab_reclaim(unsigned int flags)
{
	size_t frames = 0;
	link_t *cur;
	

	for (cur = slab_cache_list.next; cur != &slab_cache_list;
	    cur = cur->next) {
		slab_cache_t *cache = list_get_instance(cur, slab_cache_t, link);
		frames += _slab_reclaim(cache, flags);
	}
	
	
	
	return frames;
}

/* Print list of slabs
 *
 */
void slab_print_list(void)
{
	link_t *cur;
	size_t i;
	const char *name;
	RAW_U8 order;
	size_t size;
	size_t objects;
	slab_cache_t *cache;
	long allocated_slabs;
	long cached_objs;
	long allocated_objs;
	unsigned int flags;
	size_t skip = 0;

	raw_printk("[slab name       ] [size  ] [pages ] [obj/pg] [slabs ]"
	    " [cached] [alloc ] [ctl]\n");
	

	while (true) {
		/*
		 * We must not hold the slab_cache_lock spinlock when printing
		 * the statistics. Otherwise we can easily deadlock if the print
		 * needs to allocate memory.
		 *
		 * Therefore, we walk through the slab cache list, skipping some
		 * amount of already processed caches during each iteration and
		 * gathering statistics about the first unprocessed cache. For
		 * the sake of printing the statistics, we realese the
		 * slab_cache_lock and reacquire it afterwards. Then the walk
		 * starts again.
		 *
		 * This limits both the efficiency and also accuracy of the
		 * obtained statistics. The efficiency is decreased because the
		 * time complexity of the algorithm is quadratic instead of
		 * linear. The accuracy is impacted because we drop the lock
		 * after processing one cache. If there is someone else
		 * manipulating the cache list, we might omit an arbitrary
		 * number of caches or process one cache multiple times.
		 * However, we don't bleed for this algorithm for it is only
		 * statistics.
		 */

		for (i = 0, cur = slab_cache_list.next;
		    (i < skip) && (cur != &slab_cache_list);
		    i++, cur = cur->next);
		
		if (cur == &slab_cache_list) {
		
			break;
		}
		
		skip++;
		
		cache = list_get_instance(cur, slab_cache_t, link);
		
		name = cache->name;
		order = cache->order;
		size = cache->size;
		objects = cache->objects;
		allocated_slabs = atomic_get(&cache->allocated_slabs);
		cached_objs = atomic_get(&cache->cached_objs);
		allocated_objs = atomic_get(&cache->allocated_objs);
		flags = cache->flags;
		
		
		raw_printk("%-18s %8u %8u %8u %8ld %8ld %8ld %-5s\n",
		    name, size, (1 << order), objects, allocated_slabs,
		    cached_objs, allocated_objs,
		    flags & SLAB_CACHE_SLINSIDE ? "in" : "out");
	}
}

void slab_cache_init(void)
{
	size_t i;
	size_t size;


	/* Initialize magazine cache */
	_slab_cache_create(&mag_cache, "slab_magazine",
	    sizeof(slab_magazine_t) + SLAB_MAG_SIZE * sizeof(void*),
	    sizeof(uintptr_t), NULL, NULL, SLAB_CACHE_NOMAGAZINE |
	    SLAB_CACHE_SLINSIDE);

	/* Initialize slab_cache cache */
	_slab_cache_create(&slab_cache_cache, "slab_cache",
	    sizeof(slab_cache_cache), sizeof(uintptr_t), NULL, NULL,
	    SLAB_CACHE_NOMAGAZINE | SLAB_CACHE_SLINSIDE);

	/* Initialize external slab cache */
	slab_extern_cache = slab_cache_create("slab_extern", sizeof(slab_t), 0,
	    NULL, NULL, SLAB_CACHE_SLINSIDE | SLAB_CACHE_MAGDEFERRED);

	/* Initialize structures for malloc */


	for (i = 0, size = (1 << SLAB_MIN_MALLOC_W);
	    i < (SLAB_MAX_MALLOC_W - SLAB_MIN_MALLOC_W + 1);
	    i++, size <<= 1) {
		malloc_caches[i] = slab_cache_create(malloc_names[i], size, 0,
		    NULL, NULL, SLAB_CACHE_MAGDEFERRED);
	}
	
#ifdef CONFIG_DEBUG
	_slab_initialized = 1;
#endif
}

/** Enable cpu_cache
 *
 * Kernel calls this function, when it knows the real number of
 * processors. Allocate slab for cpucache and enable it on all
 * existing slabs that are SLAB_CACHE_MAGDEFERRED
 *
 */
void slab_enable_cpucache(void)
{
	slab_cache_t *slab;	
	link_t *cur;
#ifdef CONFIG_DEBUG
	_slab_initialized = 2;
#endif

	for (cur = slab_cache_list.next; cur != &slab_cache_list;
	    cur = cur->next) {
		slab = list_get_instance(cur, slab_cache_t, link);
		if ((slab->flags & SLAB_CACHE_MAGDEFERRED) !=
		    SLAB_CACHE_MAGDEFERRED)
			continue;
		
		(void) make_magcache(slab);
		slab->flags &= ~SLAB_CACHE_MAGDEFERRED;
	}
	
}

void *raw_malloc_flag(size_t size, unsigned int flags)
{
	RAW_U8 idx;
	ASSERT(_slab_initialized);
	ASSERT(size <= (1 << SLAB_MAX_MALLOC_W)); /*must be smaller than 4M*/
	
	if (size < (1 << SLAB_MIN_MALLOC_W))/*min size 16 bytes*/
		size = (1 << SLAB_MIN_MALLOC_W);
	
	idx = fnzb(size - 1) - SLAB_MIN_MALLOC_W + 1;
	
	return slab_alloc(malloc_caches[idx], flags);
}

void *raw_slab_malloc(size_t size)
{

	if (raw_int_nesting) {

		return 0;
	}
	
	return raw_malloc_flag(size, FRAME_ATOMIC);
}

void *raw_slab_realloc(void *ptr, size_t size, unsigned int flags)
{
	RAW_U8 idx;
	slab_t *slab;
	void *new_ptr;

	if (raw_int_nesting) {

		return 0;
	}

	ASSERT(_slab_initialized);
	ASSERT(size <= (1 << SLAB_MAX_MALLOC_W));
	
	if (size > 0) {
		if (size < (1 << SLAB_MIN_MALLOC_W))
			size = (1 << SLAB_MIN_MALLOC_W);
		idx = fnzb(size - 1) - SLAB_MIN_MALLOC_W + 1;
		
		new_ptr = slab_alloc(malloc_caches[idx], flags);
	} else
		new_ptr = NULL;
	
	if ((new_ptr != NULL) && (ptr != NULL)) {
		slab = obj2slab(ptr);
		raw_memcpy(new_ptr, ptr, fifo_min(size, slab->cache->size));
	}
	
	if (ptr != NULL)
		raw_slab_free(ptr);
	
	return new_ptr;
}

int raw_slab_free(void *ptr)
{
	slab_t *slab;

	if (raw_int_nesting) {

		return FAILUE;
	}
	
	if (!ptr) {
		return FAILUE;
	}
	
	slab = obj2slab(ptr);
	return _slab_free(slab->cache, ptr, slab);
}

/** @}
 */
 
