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


/* 	2013-3  Created by jorya_txj
  *	xxxxxx   please added here
  */


#include <raw_api.h>

#if (CONFIG_RAW_TASK_SEMAPHORE > 0)

/*
************************************************************************************************************************
*                                       Create a task semaphore object 
*
* Description: This function is called to create a task semphore object.
*
* Arguments  :task_obj is the address of task object want to be initialized 
*                 -----
*                 semaphore_ptr is the address of semphore object want to be initialized 
*                 -----
*                 name_ptr  is the semphore object name
*                 -----
*                 initial_count is the initial value of the semphore
*				         
* Returns			
		      RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_semaphore_create(RAW_TASK_OBJ *task_obj, RAW_SEMAPHORE *semaphore_ptr, RAW_U8 *name_ptr, RAW_U32 initial_count) 
{
	task_obj->task_semaphore_obj = semaphore_ptr;
	return raw_semaphore_create(task_obj->task_semaphore_obj, name_ptr, initial_count);
}


/*
************************************************************************************************************************
*                                       Release a task semaphore
*
* Description: This function is called to release a semphore and to wake up the highest priority task blocked on semphore if possible.
*
* Arguments  :task_obj is the address of task object want to be initialized 
*                 -----
*                 
*				         
* Returns	RAW_SEMAPHORE_OVERFLOW: semphore value excedded 0xffffffff
			RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_semaphore_put(RAW_TASK_OBJ *task_obj)
{
	return raw_semaphore_put(task_obj->task_semaphore_obj);
}

/*
************************************************************************************************************************
*                                       Get a task semaphore
*
* Description: This function is called to get a task semaphore.
*
* Arguments  : --------
*                 wait_option: 0 means return immediately if not get a task semphore.
*
*		     RAW_NO_WAIT (0x00000000)
*		     RAW_WAIT_FOREVER (0xFFFFFFFF)
*		     timeout value (0x00000001
*					      through
*						0xFFFFFFFE)   
* Returns		
*					RAW_SUCCESS : Get semphore success.
*               RAW_BLOCK_ABORT: semphore is aborted by other task or ISR.
*               RAW_NO_PEND_WAIT: semphore is not got and option is RAW_NO_WAIT.
*               RAW_SCHED_DISABLE: semphore is locked ant task is not allowed block.
*               RAW_BLOCK_DEL: if this mutex is deleted
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_semaphore_get(RAW_TICK_TYPE wait_option)
{
	return raw_semaphore_get(raw_task_active->task_semaphore_obj, wait_option);
}

/*
************************************************************************************************************************
*                                    Set a task semaphore count to specific semaphore
*
* Description: This function is called to set a semaphore count to specific semaphore.
*
* Arguments  :task_obj is the address of task object want to be initialized
*                 -----
*                   sem_count is semaphore count want to be set
*                 				         
* Returns			
*			RAW_SUCCESS: raw os return success.
*                   RAW_SEMAPHORE_TASK_WAITING: task is waiting for this semaphore, so semaphore count can not be set.
*					
* Note(s)  This function is normally used to reset the task semaphore count.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_semaphore_set(RAW_TASK_OBJ *task_obj, RAW_U32 sem_count)
{
	return raw_semaphore_set(task_obj->task_semaphore_obj, sem_count);
}


/*
************************************************************************************************************************
*                                       Delete a task semaphore
*
* Description: This function is called to delete a task semphore.
*
* Arguments  :task_obj is the address of task object want to be initialized.
*                				         
* Returns   RAW_SUCCESS: raw os return success
* Note(s)    task pended on this semphore will be waked up and will return RAW_BLOCK_DEL.
*					
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_semaphore_delete(RAW_TASK_OBJ *task_obj)
{
	return raw_semaphore_delete(task_obj->task_semaphore_obj);
}


#endif

