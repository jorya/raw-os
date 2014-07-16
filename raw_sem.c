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


/*
************************************************************************************************************************
*                                       Create a semaphore object 
*
* Description: This function is called to create a semphore object.
*
* Arguments  :semaphore_ptr is the address of semphore object want to be initialized 
*                 -----
*                  name_ptr  is the semphore object name
*                 -----
*                  initial_count is the initial value of the semphore
*				         
* Returns			
						RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_SEMAPHORE > 0)

RAW_U16 raw_semaphore_create(RAW_SEMAPHORE *semaphore_ptr, RAW_U8 *name_ptr, RAW_U32 initial_count)
{
	
	
	#if (RAW_SEMA_FUNCTION_CHECK > 0)
	
	if (semaphore_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (initial_count == 0xffffffff) {

		return RAW_SEMAPHORE_OVERFLOW;

	}
	
	#endif

	/*Init the list*/
	list_init(&semaphore_ptr->common_block_obj.block_list);
	
	/*Init resource*/
	semaphore_ptr->count     = initial_count;                                 
	
	semaphore_ptr->common_block_obj.name = name_ptr;  
	
	semaphore_ptr->common_block_obj.block_way = RAW_BLOCKED_WAY_PRIO;
	semaphore_ptr->semphore_send_notify = 0;
	
	semaphore_ptr->common_block_obj.object_type = RAW_SEM_OBJ_TYPE;

	TRACE_SEMAPHORE_CREATE(raw_task_active, semaphore_ptr);
	
	return RAW_SUCCESS;

}


RAW_U16 semaphore_put(RAW_SEMAPHORE *semaphore_ptr, RAW_U8 opt_wake_all)
{
	LIST *block_list_head;
	
	RAW_SR_ALLOC();
	
	RAW_CRITICAL_ENTER();

	if (semaphore_ptr->common_block_obj.object_type != RAW_SEM_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &semaphore_ptr->common_block_obj.block_list;
	
	/*if no block task on this list just return*/
	if (is_list_empty(block_list_head)) {        
	    
		if (semaphore_ptr->count == RAW_SEMAPHORE_COUNT) {

			RAW_CRITICAL_EXIT();
			TRACE_SEMAPHORE_OVERFLOW(raw_task_active, semaphore_ptr);
			return RAW_SEMAPHORE_OVERFLOW;

		}
		/*increase resource*/
		semaphore_ptr->count++;                                      
	    
		RAW_CRITICAL_EXIT();
		
		/*if semphore is registered with notify function just call it*/		
		if (semaphore_ptr->semphore_send_notify) {

			semaphore_ptr->semphore_send_notify(semaphore_ptr);	
		}

		TRACE_SEMAPHORE_COUNT_INCREASE(raw_task_active, semaphore_ptr);
		return RAW_SUCCESS;
	}

	/*wake all the task blocked on this semphore*/
	if (opt_wake_all) {

		while (!is_list_empty(block_list_head)) {
			
			raw_wake_object(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list));

			
			TRACE_SEM_WAKE_TASK(raw_task_active, raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list), opt_wake_all);
			
		}

	}

	else {
		
		/*Wake up the highest priority task block on the semaphore*/
		raw_wake_object(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list));

		TRACE_SEM_WAKE_TASK(raw_task_active, raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list), opt_wake_all);
		
	}
	
	RAW_CRITICAL_EXIT();

	raw_sched();    

	return RAW_SUCCESS;



}



/*
************************************************************************************************************************
*                                       Release a semaphore(wake up one blocked task)
*
* Description: This function is called to release a semphore and to wake up the highest priority task blocked on semphore if possible.
*
* Arguments  :semaphore_ptr is the address of semphore object want to be initialized 
*                 -----
*                 
*				         
* Returns          RAW_SEMAPHORE_OVERFLOW: semphore value excedded 0xffffffff
*                      RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_semaphore_put(RAW_SEMAPHORE *semaphore_ptr)
{

	#if (RAW_SEMA_FUNCTION_CHECK > 0)
	
	if (semaphore_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
	
	if (raw_int_nesting && raw_sched_lock) {
		return int_msg_post(RAW_TYPE_SEM, semaphore_ptr, 0, 0, 0, 0);
	}
	
	#endif
	
	return semaphore_put(semaphore_ptr, WAKE_ONE_SEM);
}


/*
************************************************************************************************************************
*                                       Notify function call back and release a semaphore. 
*
* Description: This function is called to call back a registered notify function.
*
* Arguments  :semaphore_ptr is the address of semphore object want to be initialized 
*                 -----
*                 
*				         
* Returns       RAW_SEMAPHORE_OVERFLOW: semphore value excedded 0xffffffff
*                   RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_semaphore_put_notify(RAW_SEMAPHORE *semaphore_ptr)
{

	#if (RAW_SEMA_FUNCTION_CHECK > 0)
	
	if (semaphore_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
	
	if (raw_int_nesting) {
		return int_msg_post(RAW_TYPE_SEM, semaphore_ptr, 0, 0, 0, 0);
	}
	
	#endif
	
	return semaphore_put(semaphore_ptr, WAKE_ONE_SEM);
}



/*
************************************************************************************************************************
*                                       Release a semaphore(wake up all blocked task)
*
* Description: This function is called to release a semphore and to wake up all tasks blocked on semphore if possible.
*
* Arguments  :semaphore_ptr is the address of semphore object want to be initialized 
*                 -----
*                 
*				         
* Returns			RAW_SEMAPHORE_OVERFLOW: semphore value excedded 0xffffffff
						RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_semaphore_put_all(RAW_SEMAPHORE *semaphore_ptr)
{

	#if (RAW_SEMA_FUNCTION_CHECK > 0)
	
	if (semaphore_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	#endif


	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting) {
		return int_msg_post(RAW_TYPE_SEM_ALL, semaphore_ptr, 0, 0, 0, 0);
	}
	
	#endif
	
	return semaphore_put(semaphore_ptr, WAKE_ALL_SEM);
	
}



/*
************************************************************************************************************************
*                                    Register notify function to semaphore
*
* Description: This function is called to Register notify function to semphore.
*
* Arguments  :semaphore_ptr is the address of the semphore object
*                 -----
*                   notify_function is the function to be called whennever put semphore.
*                 
*                 
*				         
* Returns			
*			RAW_SUCCESS: raw os return success
*					
* Note(s)  This function is normally used to implement pending on multi object function.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_semphore_send_notify(RAW_SEMAPHORE *semaphore_ptr, SEMPHORE_SEND_NOTIFY notify_function)
{
	RAW_SR_ALLOC();
	
	#if (RAW_SEMA_FUNCTION_CHECK > 0)

	if (semaphore_ptr == 0) {

		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}

	#endif
	
	RAW_CRITICAL_ENTER();

	if (semaphore_ptr->common_block_obj.object_type != RAW_SEM_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}
	
	semaphore_ptr->semphore_send_notify = notify_function;
	RAW_CRITICAL_EXIT();
	
	return RAW_SUCCESS;
}



/*
************************************************************************************************************************
*                                       Get a semaphore
*
* Description: This function is called to get a semaphore.
*
* Arguments  :semaphore_ptr is the address of semphore object want to be initialized 
*                  -----
*                 wait_option: 0 means return immediately if not get semphore
*
*				       RAW_NO_WAIT (0x00000000)
*						RAW_WAIT_FOREVER (0xFFFFFFFF)
*						timeout value (0x00000001
*							  					through
*												0xFFFFFFFE)   
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
RAW_U16 raw_semaphore_get(RAW_SEMAPHORE *semaphore_ptr, RAW_TICK_TYPE wait_option)
{

	RAW_U16 error_status;

	RAW_SR_ALLOC();

	#if (RAW_SEMA_FUNCTION_CHECK > 0)

	if (semaphore_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}

	
	
	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}

	#endif
	
	RAW_CRITICAL_ENTER();

	if (semaphore_ptr->common_block_obj.object_type != RAW_SEM_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}
	
	if (semaphore_ptr->count) {                      
		semaphore_ptr->count--;                                       

		RAW_CRITICAL_EXIT();
		TRACE_SEMAPHORE_GET_SUCCESS(raw_task_active, semaphore_ptr);		
		return RAW_SUCCESS;
	}
	
	/*Cann't get semphore, and return immediately if wait_option is  RAW_NO_WAIT*/
	if (wait_option == RAW_NO_WAIT) { 

		RAW_CRITICAL_EXIT();
		return RAW_NO_PEND_WAIT;
	}      
	
	SYSTEM_LOCK_PROCESS();

	raw_pend_object((RAW_COMMON_BLOCK_OBJECT  *)semaphore_ptr, raw_task_active, wait_option);
	RAW_CRITICAL_EXIT();

	TRACE_SEMAPHORE_GET_BLOCK(raw_task_active, semaphore_ptr, wait_option);
	
	raw_sched(); 
	
	error_status = block_state_post_process(raw_task_active, 0);
	return error_status;

}

/*
************************************************************************************************************************
*                                    Set a semaphore count to specific semaphore
*
* Description: This function is called to set a semaphore count to specific semaphore.
*
* Arguments  :semaphore_ptr is the address of the semphore object
*                 -----
*                   sem_count is semaphore count want to be set
*                 
*                 
*				         
* Returns			
*			RAW_SUCCESS: raw os return success.
*                   RAW_SEMAPHORE_TASK_WAITING: task is waiting for this semaphore, so semaphore count can not be set.
*					
* Note(s)  This function is normally used to reset the semaphore count.
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_SEMAPHORE_SET > 0)

RAW_U16 raw_semaphore_set(RAW_SEMAPHORE *semaphore_ptr,  RAW_U32 sem_count)
{
	LIST *block_list_head;
	
	RAW_SR_ALLOC();

	block_list_head = &semaphore_ptr->common_block_obj.block_list;

	#if (RAW_SEMA_FUNCTION_CHECK > 0)

	if (semaphore_ptr == 0) {

		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}

	#endif

	RAW_CRITICAL_ENTER();

	if (semaphore_ptr->common_block_obj.object_type != RAW_SEM_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}
	
	if (semaphore_ptr->count) { 
		
		semaphore_ptr->count = sem_count;                                   
	} 

	else {

		if (is_list_empty(block_list_head)) {

			semaphore_ptr->count = sem_count; 
		}

		else {

			RAW_CRITICAL_EXIT();
			return RAW_SEMAPHORE_TASK_WAITING;
		}

	}

	RAW_CRITICAL_EXIT();

	return RAW_SUCCESS;

}

#endif

/*
************************************************************************************************************************
*                                       Delete a semaphore
*
* Description: This function is called to delete a semphore.
*
* Arguments  :semaphore_ptr is the address of semphore object.
*                
*                
*				         
* Returns		RAW_SUCCESS: raw os return success
* Note(s)    Any task pended on this semphore will be waked up and will return RAW_BLOCK_DEL.
*					
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_SEMAPHORE_DELETE > 0)
RAW_U16 raw_semaphore_delete(RAW_SEMAPHORE *semaphore_ptr)
{
	LIST *block_list_head;
	
	RAW_SR_ALLOC();

	#if (RAW_SEMA_FUNCTION_CHECK > 0)
	
	if (semaphore_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}

	#endif
	
	RAW_CRITICAL_ENTER();

	if (semaphore_ptr->common_block_obj.object_type != RAW_SEM_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &semaphore_ptr->common_block_obj.block_list;
	
	semaphore_ptr->common_block_obj.object_type = 0;
	/*All task blocked on this queue is waken up*/
	while (!is_list_empty(block_list_head)) {
		delete_pend_obj(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list));	
	}                             

	RAW_CRITICAL_EXIT();

	TRACE_SEMAPHORE_DELETE(raw_task_active, semaphore_ptr);
	
	raw_sched(); 
	
	return RAW_SUCCESS;
}
#endif

#endif

