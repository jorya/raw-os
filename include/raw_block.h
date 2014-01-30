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


/* 	2012-4  Created by jorya_txj
  *	xxxxxx   please added here
  */
  

#ifndef RAW_BLOCK_H
#define RAW_BLOCK_H

typedef struct MEM_POOL
{
	RAW_COMMON_BLOCK_OBJECT             common_block_obj;

	/* Define the number of available memory blocks in the pool.  */
	RAW_U32                             raw_block_pool_available;
	RAW_U32                             block_size;

	/* Define the head pointer of the available block pool.  */
	RAW_U8                              *raw_block_pool_available_list;

} MEM_POOL;


RAW_U16  raw_block_pool_create(MEM_POOL *pool_ptr, RAW_U8  *name_ptr, RAW_U32  block_size, RAW_VOID  *pool_start, RAW_U32  pool_size);
RAW_U16 raw_block_allocate(MEM_POOL *pool_ptr, RAW_VOID **block_ptr);
RAW_U16 raw_block_release(MEM_POOL *pool_ptr, RAW_VOID *block_ptr);


#endif

