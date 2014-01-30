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




/** Align to the nearest lower address.
 *
 * @param s Address or size to be aligned.
 * @param a Size of alignment, must be power of 2.
 */
#define ALIGN_DOWN(s, a)  ((s) & ~((a) - 1))


/** Align to the nearest higher address.
 *
 * @param s Address or size to be aligned.
 * @param a Size of alignment, must be power of 2.
 */
#define ALIGN_UP(s, a)  (((s) + ((a) - 1)) & ~((a) - 1))

#ifndef ATOMIC_TYPE
#define ATOMIC_TYPE

typedef RAW_U32 atomic_count_t;

typedef struct {
	volatile atomic_count_t count;
} atomic_t;

RAW_INLINE void atomic_set(atomic_t *val, atomic_count_t i)
{
	val->count = i;
}

RAW_INLINE atomic_count_t atomic_get(atomic_t *val)
{
	return val->count;
}

#endif


#define SLAB_CPU_32 1

#if (SLAB_CPU_32 > 0)

#define fnzb(arg)  fnzb32(arg)

#else

#define fnzb(arg)  fnzb64(arg)
#endif


/** Return position of first non-zero bit from left (32b variant).
 *
 * @return 0 (if the number is zero) or [log_2(arg)].
 *
 */
RAW_INLINE RAW_U8 fnzb32(RAW_U32 arg)
{
	RAW_U8 n = 0;
	
	if (arg >> 16) {
		arg >>= 16;
		n += 16;
	}
	
	if (arg >> 8) {
		arg >>= 8;
		n += 8;
	}
	
	if (arg >> 4) {
		arg >>= 4;
		n += 4;
	}
	
	if (arg >> 2) {
		arg >>= 2;
		n += 2;
	}
	
	if (arg >> 1)
		n += 1;
	
	return n;
}

/** Return position of first non-zero bit from left (64b variant).
 *
 * @return 0 (if the number is zero) or [log_2(arg)].
 *
 */
RAW_INLINE RAW_U8 fnzb64(RAW_U64 arg)
{
	RAW_U8 n = 0;
	
	if (arg >> 32) {
		arg >>= 32;
		n += 32;
	}
	
	return n + fnzb32((RAW_U32) arg);
}


#ifndef OVERLAPS
#define OVERLAPS
/** Return true if the intervals overlap.
 *
 * @param s1  Start address of the first interval.
 * @param sz1 Size of the first interval.
 * @param s2  Start address of the second interval.
 * @param sz2 Size of the second interval.
 *
 */
RAW_INLINE int overlaps(RAW_U64 s1, RAW_U64 sz1, RAW_U64 s2,
    RAW_U64 sz2)
{
	RAW_U64 e1 = s1 + sz1;
	RAW_U64 e2 = s2 + sz2;
	
	return ((s1 < e2) && (s2 < e1));
}

/** Return true if the second interval is within the first interval.
 *
 * @param s1  Start address of the first interval.
 * @param sz1 Size of the first interval.
 * @param s2  Start address of the second interval.
 * @param sz2 Size of the second interval.
 *
 */
RAW_INLINE int iswithin(RAW_U64 s1, RAW_U64 sz1, RAW_U64 s2,
    RAW_U64 sz2)
{
	RAW_U64 e1 = s1 + sz1;
	RAW_U64 e2 = s2 + sz2;
	
	return ((s1 <= s2) && (e1 >= e2));
}

#endif

#define min3(a, b, c)  ((a) < (b) ? (min(a, c)) : (min(b, c)))
#define max3(a, b, c)  ((a) > (b) ? (max(a, c)) : (max(b, c)))

#define SIZE2KB(size)  ((size) >> 10)
#define SIZE2MB(size)  ((size) >> 20)

#define KB2SIZE(kb)  ((kb) << 10)
#define MB2SIZE(mb)  ((mb) << 20)

#define PAGE_SIZE_SLAB	FRAME_SIZE

#ifndef NULL
#define NULL  ((void *) 0)
#endif

#define false  0
#define true   1

typedef void (* function)();

#define panic	vc_port_printf

#define ASSERT  RAW_ASSERT

