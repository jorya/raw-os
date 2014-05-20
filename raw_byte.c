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

#if (CONFIG_RAW_BYTE > 0)


/*
************************************************************************************************************************
*                                       Create a  byte pool for allocating memory
*
* Description: This function is called to create a byte pool .
*
* Arguments  :pool_ptr is the address of this pool
*                   --------------
*                   name_ptr is the name of this pool               
*                   -------------------
*						pool_start is the pool start address
*						-------------------
*                   pool_size is the allocating size of a block
*                    -------------------				       
*
* Returns	  RAW_INVALID_ALIGN:pool_start or pool_size not be pointed aligned!!! 
*				  RAW_SUCCESS: raw os return success
* Note(s)  This methods will  cause memory fragmention and not suggested to be used by time critical condition.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_byte_pool_create(RAW_BYTE_POOL_STRUCT *pool_ptr, RAW_U8 *name_ptr, RAW_VOID *pool_start, RAW_U32 pool_size)
{

	RAW_U8  *block_ptr;                  /* Working block pointer       */
	RAW_U8   byte_align_mask;

	#if (RAW_BYTE_FUNCTION_CHECK > 0)

	if (pool_ptr == 0) {

		return RAW_NULL_POINTER;
	}

	if (pool_start == 0) {

		return RAW_NULL_POINTER;
	}

	/* Check for invalid pool size.  */
	if (pool_size < RAW_BYTE_POOL_MIN) {

		return RAW_BYTE_SIZE_ERROR;

	}

	#endif

	byte_align_mask = sizeof(void *) - 1u;

	/*pool_start needs 4 bytes aligned*/
	if (((RAW_U32)pool_start & byte_align_mask)){                             

		return RAW_INVALID_ALIGN;

	}

	/*pool_size needs 4 bytes aligned*/
	if ((pool_size & byte_align_mask)) {   
		
		return RAW_INVALID_ALIGN;
	}
	
	/* Setup the basic byte pool fields.  */
	pool_ptr->common_block_obj.name = name_ptr;
	pool_ptr->raw_byte_pool_search = (RAW_U8 *)pool_start;
	pool_ptr->raw_byte_pool_owner = 0;

	/*Initially, the pool will have two blocks.  One large block at the 
	   beginning that is available and a small allocated block at the end
	   of the pool that is there just for the algorithm.  Be sure to count
	   the available block's header in the available bytes count.  */
	pool_ptr->raw_byte_pool_available = pool_size - sizeof(RAW_U8 *) - sizeof(RAW_U32);
	pool_ptr->raw_byte_pool_fragments = 2u;

	/* Calculate the end of the pool's memory area.  */
	block_ptr = ((RAW_U8 *)pool_start) + (RAW_U32) pool_size;

	/* Backup the end of the pool pointer and build the pre-allocated block.  */
	block_ptr = block_ptr - sizeof(RAW_U32);
	*((RAW_U32 *)block_ptr) = RAW_BYTE_BLOCK_ALLOC;
	block_ptr = block_ptr - sizeof(RAW_U8 *);
	*((RAW_U8 * *)block_ptr) = pool_start;

	/* Now setup the large available block in the pool.  */
	*((RAW_U8  * *) pool_start) = block_ptr;
	block_ptr = (RAW_U8  *)pool_start;
	block_ptr = block_ptr + sizeof(RAW_U8 *);
	*((RAW_U32 *) block_ptr) = RAW_BYTE_BLOCK_FREE;

 	pool_ptr->common_block_obj.object_type = RAW_BYTE_OBJ_TYPE;

	return RAW_SUCCESS;
	
}


static void *raw_byte_pool_search(RAW_BYTE_POOL_STRUCT *pool_ptr, RAW_U32 memory_size)
{

	RAW_U8 *current_ptr;                /* Current block pointer      */
	RAW_U8 *next_ptr;                   /* Next block pointer         */
	RAW_U32 available_bytes;            /* Calculate bytes available  */
	RAW_U32 examine_blocks;             /* Blocks to be examined      */

	RAW_SR_ALLOC();

	/* Disable interrupts.  */
	RAW_CPU_DISABLE();

	/* First, determine if there are enough bytes in the pool.  */
	if (memory_size >= pool_ptr->raw_byte_pool_available) {

		/* Restore interrupts.  */
		RAW_CPU_ENABLE();
		/* Not enough memory, return a NULL pointer.  */
		return 0;
	}

	/* Walk through the memory pool in search for a large enough block.  */
	current_ptr = pool_ptr->raw_byte_pool_search;
	examine_blocks = pool_ptr->raw_byte_pool_fragments + 1u;
	available_bytes = 0u;
		
	do {
		
		/* Check to see if this block is free.  */
		if (*((RAW_U32 *)(current_ptr + sizeof(RAW_U8  *))) == RAW_BYTE_BLOCK_FREE) {

			/* Block is free, see if it is large enough.  */

			/* Pickup the next block's pointer.  */
			next_ptr = *((RAW_U8 * *) current_ptr);

			/* Calculate the number of byte available in this block.  */
			available_bytes = next_ptr - current_ptr - sizeof(RAW_U8  *) - sizeof(RAW_U32);

			/* If this is large enough, we are done because our first-fit algorithm
			has been satisfied!  */
			if (available_bytes >= memory_size) {

				/* Find the suitable position */
				break;
			}
						
			else {
		    
				/* Clear the available bytes variable.  */
				available_bytes = 0u;

				/* Not enough memory, check to see if the neighbor is 
				free and can be merged.  */
				if (*((RAW_U32 *)(next_ptr + sizeof(RAW_U8 *))) == RAW_BYTE_BLOCK_FREE) {

					/* Yes, neighbor block can be merged!  This is quickly accomplished
					   by updating the current block with the next blocks pointer.  */
					*((RAW_U8 * *)current_ptr) = *((RAW_U8 * *)next_ptr);

					/* Reduce the fragment number, and does not need to increase  available bytes since 
					    they are already there*/
					    
					pool_ptr->raw_byte_pool_fragments--;
						
				}
				
				else {

					/* Neighbor is not free so get to the next search point*/
					current_ptr = *((RAW_U8 * *)next_ptr);

					/* Reduse the examined block since we have skiped one search */
					if (examine_blocks) {
						examine_blocks--;
					}
				}
			}
		}
		
		else {

			/* Block is not free, move to next block.  */
			current_ptr = *((RAW_U8 * *)current_ptr);
		}

		/* finish one block search*/
		if (examine_blocks) {
			examine_blocks--;
		}

		/* Restore interrupts temporarily.  */
		RAW_CPU_ENABLE();

		/* Disable interrupts.  */
		RAW_CPU_DISABLE();

		/* Determine if anything has changed in terms of pool ownership.  */
		if (pool_ptr ->raw_byte_pool_owner != raw_task_active) {

			/* Pool changed ownership during interrupts.so we reset the search*/

			current_ptr = pool_ptr ->raw_byte_pool_search;
			examine_blocks = pool_ptr ->raw_byte_pool_fragments + 1u;

			/* Setup our ownership again.  */
			pool_ptr ->raw_byte_pool_owner = raw_task_active;
		}

	} while (examine_blocks);

	/* Determine if a block was found.  If so, determine if it needs to be
	split.  */
	if (available_bytes) {

		/* Do we need to split this block if this is big enough.*/
		if ((available_bytes - memory_size) >= ((RAW_U32) RAW_BYTE_BLOCK_MIN)) {

			/* Split the block.  */
			next_ptr = current_ptr + memory_size + sizeof(RAW_U8 *) + sizeof(RAW_U32);

			/* Setup the new free block.  */
			*((RAW_U8 * *) next_ptr) = *((RAW_U8 * *) current_ptr);
			*((RAW_U32 *) (next_ptr + sizeof(RAW_U8 *))) = RAW_BYTE_BLOCK_FREE;

			/* Increase the total fragment counter.  */
			pool_ptr->raw_byte_pool_fragments++;

			/* Update the current pointer to point at the newly created block.  */
			*((RAW_U8 * *)current_ptr) = next_ptr;

			/* Set available equal to memory size for subsequent calculation.  */
			available_bytes = memory_size;
		}

			/* In any case, mark the current block as allocated.  */
			*((RAW_U32 *)(current_ptr + sizeof(RAW_U8  *))) = RAW_BYTE_BLOCK_ALLOC;

			/* Reduce the number of available bytes in the pool.  */
			pool_ptr->raw_byte_pool_available =  pool_ptr ->raw_byte_pool_available - available_bytes
			                   - sizeof(RAW_U8 *) - sizeof(RAW_U32);

			/* Adjust the pointer for the application.  */
			current_ptr = current_ptr + sizeof(RAW_U8 *) + sizeof(RAW_U32);

	}
		
	else {

		/* Set current pointer to NULL to indicate nothing was found. */
		current_ptr = 0;
	}


	/* Restore interrupts temporarily.  */
	RAW_CPU_ENABLE();

	/* Return the searched result*/
	return current_ptr;
		
}


/*
************************************************************************************************************************
*                                       Allocating byte memory from pool
*
* Description: This function is called to allocate memory from pool
*
* Arguments  :pool_ptr is the address of the pool
*                   ---------------------         
*                   memory_ptr is the address of pointer, and it will be filled the allocating block address          
*                   ---------------------
*				       memory_size is the size you want to allocate
*                  ---------------------
*
* Returns			RAW_NO_MEMORY: No block memory available.
					RAW_SUCCESS: raw os return success
* Note(s)       If  *memory_ptr is 0 which means no memory available now.This methods will cause fragmention.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_byte_allocate(RAW_BYTE_POOL_STRUCT *pool_ptr, RAW_VOID **memory_ptr, RAW_U32 memory_size)
{

	RAW_U16        status;                 /* Return status              */
	RAW_U8         *work_ptr;               /* Working byte pointer       */
	RAW_TASK_OBJ   *current_work_task;
	RAW_U8         byte_align_mask;

	RAW_SR_ALLOC();

	#if (RAW_BYTE_FUNCTION_CHECK > 0)

	if (pool_ptr == 0) {
			
		return RAW_NULL_POINTER;
	}

	if (memory_ptr == 0) {

		return RAW_NULL_POINTER;
	}
	 
	#endif

	if (pool_ptr->common_block_obj.object_type != RAW_BYTE_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}

	byte_align_mask = sizeof(void *) - 1u;
	
	/* align the memory size to pointer align*/

	memory_size = ((memory_size & (~byte_align_mask)) + sizeof(void *));

	current_work_task = raw_task_active;

	/* Disable interrupts.  */
	RAW_CPU_DISABLE();

	/* Loop to handle cases where the owner of the pool changed.  */
	do {

		/* Indicate that this thread is the current owner.  */
		pool_ptr->raw_byte_pool_owner = current_work_task;

		/* Restore interrupts.  */
		RAW_CPU_ENABLE();

		/*Search for free memory*/
		work_ptr = raw_byte_pool_search(pool_ptr, memory_size);

		/* Disable interrupts.  */
		RAW_CPU_DISABLE();

	/*if raw_byte_pool_owner changed and we have not got memory yet, continue tom do search*/
	} while ((!work_ptr) && (pool_ptr ->raw_byte_pool_owner != current_work_task));

	/* Determine if memory was found.  */
	if (work_ptr) {

		/* Copy the pointer into the return destination.  */
		*memory_ptr = (RAW_U8 *) work_ptr;

		/* Set the status to success.  */
		status = RAW_SUCCESS;

	}

	else {
		
		*memory_ptr = 0;

		/* Set the status to RAW_NO_MEMORY.  */
		status = RAW_NO_MEMORY; 

		TRACE_BYTE_NO_MEMORY(raw_task_active, pool_ptr);
	}

	RAW_CPU_ENABLE();

	return status;
}


/*
************************************************************************************************************************
*                                       Release byte memory from pool
*
* Description: This function is called to allocate memory from pool
*
* Arguments  : block_ptr is the address want to return to memory pool.          
*                   ---------------------
*				         
* Returns		
*						RAW_SUCCESS: raw os return success
* Note(s)     This methods will not cause fragmention. 
*
*             
************************************************************************************************************************
*/
RAW_U16  raw_byte_release(RAW_BYTE_POOL_STRUCT *pool_ptr, void *memory_ptr)
{
	RAW_U8  *work_ptr;           /* Working block pointer      */

	RAW_SR_ALLOC();


	#if (RAW_BYTE_FUNCTION_CHECK > 0)

	if (pool_ptr == 0) {

		return RAW_NULL_POINTER;
	}

	if (memory_ptr == 0) {

		return RAW_NULL_POINTER;
	}
		
	#endif

	if (pool_ptr ->common_block_obj.object_type != RAW_BYTE_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}
	
	/* Back off the memory pointer to pickup its header.  */
	work_ptr = (RAW_U8 *)memory_ptr - sizeof(RAW_U8 *) - sizeof(RAW_U32);

	/* Disable interrupts.  */
	RAW_CPU_DISABLE();

	/* Indicate that this thread is the current owner.  */
	pool_ptr->raw_byte_pool_owner = raw_task_active;

	/* Release the memory.*/
	*((RAW_U32 *)(work_ptr + sizeof(RAW_U8 *))) = RAW_BYTE_BLOCK_FREE;

	/* Update the number of available bytes in the pool. */
	pool_ptr->raw_byte_pool_available =  
	pool_ptr->raw_byte_pool_available + (*((RAW_U8 * *)(work_ptr)) - work_ptr);

	/* Set the pool search value appropriately. */
	pool_ptr->raw_byte_pool_search = work_ptr;

	RAW_CPU_ENABLE();

	return RAW_SUCCESS;
		
}


#endif


