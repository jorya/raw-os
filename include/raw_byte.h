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


#ifndef RAW_BYTE_H
#define RAW_BYTE_H


typedef struct RAW_BYTE_POOL_STRUCT
{
	RAW_COMMON_BLOCK_OBJECT            common_block_obj;

	/* Define the number of available bytes in the pool.  */
	RAW_U32                            raw_byte_pool_available;

	/* Define the number of fragments in the pool.  */
	RAW_U32                            raw_byte_pool_fragments;

	/* Define the search pointer used for initial searching for memory
	in a byte pool.  */
	RAW_U8                             *raw_byte_pool_search;

	/*blocked task on this byte pool*/
	RAW_TASK_OBJ                       *raw_byte_pool_owner;

} RAW_BYTE_POOL_STRUCT;


#define  RAW_BYTE_BLOCK_ALLOC    0xABCDDCABUL
#define  RAW_BYTE_BLOCK_FREE     0xFEFDECDBUL
#define  RAW_BYTE_BLOCK_MIN      20u
#define  RAW_BYTE_POOL_MIN       100u

RAW_U16 raw_byte_pool_create(RAW_BYTE_POOL_STRUCT *pool_ptr, RAW_U8 *name_ptr, RAW_VOID *pool_start, RAW_U32 pool_size);

RAW_U16 raw_byte_allocate(RAW_BYTE_POOL_STRUCT *pool_ptr, RAW_VOID **memory_ptr, RAW_U32 memory_size);

RAW_U16 raw_byte_release(RAW_BYTE_POOL_STRUCT *pool_ptr, void  *memory_ptr);


#endif

