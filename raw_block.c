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
  

#include <raw_api.h>

#if (CONFIG_RAW_BLOCK > 0)

/*
************************************************************************************************************************
*                                       Create a  block pool for allocating memory
*
* Description: This function is called to create a block pool .
* 
* Arguments:  pool_ptr is the address of this pool
*                    --------------
*                name_pr is the name of this pool               
*                    -------------------
*                    block_size is the allocating size of a block
*                    -------------------
*                    pool_start is the pool start address
*                    ---------------------------------
*                    pool_size  is the size of this pool
*
* Returns        RAW_INVALID_ALIGN:  block_size or pool_start or pool_size not be pointed aligned!!!
*                    RAW_BLOCK_SIZE_ERROR: Must at least have 2 blocks.
*                    RAW_SUCCESS: raw os return success
* Note(s)        block_size and pool_start and pool_size must be pointed aligned!!!.This methods will not cause fragmention.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_block_pool_create(MEM_POOL *pool_ptr, RAW_U8 *name_ptr, RAW_U32 block_size, RAW_VOID *pool_start, RAW_U32 pool_size)
{

	RAW_U32       blocks;                     /* Number of blocks in pool    */
	RAW_U8        *block_ptr;                  /* Working block pointer       */
	RAW_U8        *next_block_ptr;             /* Next block pointer          */
	RAW_U8        *end_of_pool;                /* End of pool area            */
	RAW_U8 			block_align_mask;
	
	#if (RAW_BLOCK_FUNCTION_CHECK > 0)
	/* Check for invalid pool size.  */

	if (pool_size < (block_size +  block_size) ) {

		return RAW_BLOCK_SIZE_ERROR;
	}

	if (pool_ptr == 0) {

		return RAW_NULL_OBJECT;
	}

	if (pool_start == 0) {

		return RAW_NULL_POINTER;
	}

	#endif

	block_align_mask = sizeof(void *) - 1u;

	if (((RAW_U32)pool_start & block_align_mask)){                             

		return RAW_INVALID_ALIGN;

	}

	if ((pool_size & block_align_mask)) {   

		return RAW_INVALID_ALIGN;
	}

	if ((block_size & block_align_mask)) {   

		return RAW_INVALID_ALIGN;
	}
	
	/* Setup the basic block pool fields.  */
	pool_ptr->common_block_obj.name = name_ptr;
	
	/* Calculate the end of the pool's memory area.  */
	end_of_pool = (RAW_U8  *)pool_start + pool_size;

	/* Walk through the pool area, setting up the available block list.  */
	blocks = 0u;
	block_ptr = (RAW_U8  *)pool_start;
	next_block_ptr = block_ptr + block_size;
	
	while (next_block_ptr <= end_of_pool) {

		blocks++;

		if (next_block_ptr == end_of_pool) {

			break;
		}

		/* Setup the link to the next block.  */
		*((RAW_U8 * *) block_ptr) = next_block_ptr;

		/* Advance to the next block.  */
		block_ptr = next_block_ptr;

		/* Update the next block pointer.  */
		next_block_ptr = block_ptr + block_size;
	}

	/* Set the last block's forward pointer to NULL.  */
	*((RAW_U8 * *)block_ptr) = 0;

	/* Save the remaining information in the pool control block.  */
	pool_ptr->raw_block_pool_available = blocks;

	pool_ptr->block_size = block_size;
	
	pool_ptr ->raw_block_pool_available_list = (RAW_U8  *)pool_start;

	pool_ptr ->common_block_obj.object_type = RAW_BLOCK_OBJ_TYPE;

	TRACE_BLOCK_POOL_CREATE(raw_task_active, pool_ptr);
	
	return RAW_SUCCESS;
	
}


/*
************************************************************************************************************************
*                                       Allocating block memory from pool
*
* Description: This function is called to allocate memory from pool
*
* Arguments  :pool_ptr is the address of the pool
*                   ---------------------         
*                   block_ptr is the address of pointer, and it will be filled the allocating block address          
*                   ---------------------
*
* Returns      RAW_NO_MEMORY: No block memory is available.
*                  RAW_SUCCESS: raw os return success
* Note(s)       If *block_ptr is 0 which means no memory available now.This methods will not cause fragmention.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_block_allocate(MEM_POOL *pool_ptr, RAW_VOID **block_ptr)
{
	
	RAW_U16				status; 							

	RAW_U8		*work_ptr; 						

	RAW_SR_ALLOC();

	#if (RAW_BLOCK_FUNCTION_CHECK > 0)
	 
	if (pool_ptr == 0) {
		return RAW_NULL_OBJECT;
	}
	
	if (block_ptr == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	#endif

	if (pool_ptr->common_block_obj.object_type != RAW_BLOCK_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}

	RAW_CPU_DISABLE();

	/* Determine if there is an available block.  */
	if (pool_ptr->raw_block_pool_available) {

		/* Yes, a block is available.  Decrement the available count.  */
		pool_ptr->raw_block_pool_available--;

		/* Pickup the current block pointer.  */
		work_ptr = pool_ptr->raw_block_pool_available_list;

		/* Return the first available block to the caller.  */
		*((RAW_U8 **)block_ptr) = work_ptr;

		/* Modify the available list to point at the next block in the pool. */
		pool_ptr->raw_block_pool_available_list = *((RAW_U8 **)work_ptr);

		/* Set status to success.  */
		status =  RAW_SUCCESS;
	}

	/*if no block memory is available then just return*/
	else {

		*((RAW_U8 **)block_ptr) = 0;
		status = RAW_NO_MEMORY;
		TRACE_BLOCK_NO_MEMORY(raw_task_active, pool_ptr);
	}

	RAW_CPU_ENABLE();
	
	return status;

}




/*
************************************************************************************************************************
*                                       Release block memory from pool
*
* Description: This function is called to release memory from pool
*
* Arguments  : block_ptr is the address want to return to memory pool.          
*                   ---------------------
*				         
* Returns		
*                  RAW_SUCCESS: raw os return success
* Note(s)     This methods will not cause fragmention. 
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_block_release(MEM_POOL *pool_ptr, RAW_VOID *block_ptr)
{
	RAW_U8	*work_ptr;   
	
	RAW_SR_ALLOC();

	#if (RAW_BLOCK_FUNCTION_CHECK > 0)
	 
	if (block_ptr == 0) {
		return RAW_NULL_OBJECT;
	}
	
	if (pool_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	#endif

	if (pool_ptr->common_block_obj.object_type != RAW_BLOCK_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}

	RAW_CPU_DISABLE();
	
	work_ptr = ((RAW_U8 *) block_ptr);

	/* Put the block back in the available list.  */
	*((RAW_U8 **)work_ptr) = pool_ptr->raw_block_pool_available_list;

	/* Adjust the head pointer.  */
	pool_ptr->raw_block_pool_available_list = work_ptr;        

	/* Increment the count of available blocks.  */
	pool_ptr->raw_block_pool_available++;
	
	RAW_CPU_ENABLE();

	/* Return completion status.  */
	return RAW_SUCCESS;
	
}

#endif

