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




typedef struct {
	atomic_t val;
	const char *name;
} spinlock_t;

#define SPINLOCK_DECLARE(lock_name)  spinlock_t lock_name

/*Non-SMP architecture.*/
#define SPINLOCK_EXTERN(name)

#define SPINLOCK_INITIALIZE(name)
#define SPINLOCK_STATIC_INITIALIZE(name)

#define SPINLOCK_INITIALIZE_NAME(name, desc_name)
#define SPINLOCK_STATIC_INITIALIZE_NAME(name, desc_name)

#define ASSERT_SPINLOCK(expr, lock)  ASSERT(expr)
#define ASSERT_IRQ_SPINLOCK(expr, irq_lock) \
	ASSERT_SPINLOCK(expr, NULL)

#define spinlock_initialize(lock, name)

#define spinlock_lock(lock)     
#define spinlock_trylock(lock)  
#define spinlock_locked(lock)	
#define spinlock_unlocked(lock)
#define spinlock_unlock(lock)


typedef char bool;

typedef struct {
	SPINLOCK_DECLARE(lock);  /**< Spinlock */
	bool  guard;              /**< Flag whether ipl is valid */
	
} irq_spinlock_t;

#define IRQ_SPINLOCK_DECLARE(lock_name)  irq_spinlock_t lock_name

#define IRQ_SPINLOCK_STATIC_INITIALIZE_NAME(lock_name, desc_name) 
	

RAW_INLINE void irq_spinlock_initialize(irq_spinlock_t *lock, const char *name)
{
	

}

RAW_INLINE void irq_spinlock_lock(irq_spinlock_t *lock, bool irq_dis)
{

}

RAW_INLINE void irq_spinlock_unlock(irq_spinlock_t *lock, bool irq_res)
{


}

