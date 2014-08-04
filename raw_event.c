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

#if (CONFIG_RAW_EVENT > 0)

/*
************************************************************************************************************************
*                                       Create an event
*
* Description: This service creates a group of 32 event flags.  Each event flag is represented by a single bit.
*
* Arguments  :event_ptr is the address of event object
*                    -----
*                
*				         
*				         
* Returns   RAW_SUCCESS : create event success.
*
* Note(s)    :Normally flags_init is 0.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_event_create(RAW_EVENT *event_ptr, RAW_U8 *name_ptr, RAW_U32 flags_init)
{
	
	#if (RAW_EVENT_FUNCTION_CHECK > 0)
	
	if (event_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	#endif

	/*Init the list*/
	list_init(&event_ptr->common_block_obj.block_list);
	event_ptr->common_block_obj.block_way = RAW_BLOCKED_WAY_PRIO;
	event_ptr->common_block_obj.name = name_ptr;  
	event_ptr->flags = flags_init;
	event_ptr->common_block_obj.object_type = RAW_EVENT_OBJ_TYPE;

	TRACE_EVENT_CREATE(raw_task_active, event_ptr, name_ptr, flags_init);
		
	return RAW_SUCCESS;
}



/*
************************************************************************************************************************
*                                      Get an event
*
* Description: This service retrieves event flags from the specified event flags group.
*						Each event flags group contains 32 event flags. Each flag is represented
*						by a single bit. This service can retrieve a variety of event flag
*						combinations, as selected by the input parameters.
*
* Arguments  :event_ptr: is the address of event object
*                    -----
*                   requested_flags: is the 32-bit unsigned variable that represents the requested event flags.
*				        -----
*                   get_option: is the option specifies whether all or any of the requested event flags are required. The following are valid
*						selections:
*						               RAW_AND 
*						               RAW_AND_CLEAR 
*							        RAW_OR 
*							        RAW_OR_CLEAR 
*							Selecting RAW_AND or RAW_AND_CLEAR
*							specifies that all event flags must be present in
*							the group. Selecting RAW_OR or RAW_OR_CLEAR
*							specifies that any event flag is satisfactory. Event
*							flags that satisfy the request are cleared (set to
*							zero) if RAW_AND_CLEAR or RAW_OR_CLEAR are specified.
*							-----
*							wait_option: is  how the service behaves if the selected
*							event flags are not set. The wait options are
*							defined as follows:
*							RAW_NO_WAIT (0x00000000)
*							RAW_WAIT_FOREVER (0xFFFFFFFF)
*							timeout value (0x00000001
*							through
*							0xFFFFFFFE)
*                actual_flags_ptr:will be filled the actual flags when the function is returned.
*                      
* Returns   RAW_SUCCESS : Get event success.
*              RAW_BLOCK_ABORT: event is aborted by other task or ISR.
*              RAW_NO_PEND_WAIT: event is not got and option is RAW_NO_WAIT.
*              RAW_SCHED_DISABLE: system is locked ant task is not allowed block.
*              RAW_BLOCK_DEL: if this event is deleted
* Note(s)  :RAW_STATE_UNKNOWN wrong task state, probally sysytem error.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_event_get(RAW_EVENT *event_ptr, RAW_U32 requested_flags, RAW_U8 get_option, RAW_U32 *actual_flags_ptr, RAW_TICK_TYPE wait_option)
{
   	RAW_U16 error_status;
   
	RAW_U8 status;
	RAW_SR_ALLOC();

	#if (RAW_EVENT_FUNCTION_CHECK > 0)

	if (event_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
		
	}

	if ((get_option  != RAW_AND) && (get_option  != RAW_OR) && (get_option  != RAW_AND_CLEAR) && (get_option  != RAW_OR_CLEAR)) {

		return RAW_NO_THIS_OPTION;
	}
	
	#endif

	
	RAW_CRITICAL_ENTER();

	if (event_ptr->common_block_obj.object_type != RAW_EVENT_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		
		return RAW_ERROR_OBJECT_TYPE;
	}

	/*if option is and flag*/
	if (get_option & RAW_FLAGS_AND_MASK) {

		if ((event_ptr->flags & requested_flags) == requested_flags) {

			status = RAW_TRUE;
		}
		
		else {
			status =  RAW_FALSE;
		}
		
	}
	
	/*if option is or flag*/
	else {

		if (event_ptr->flags & requested_flags) {

			status =  RAW_TRUE;
		}
		
		else {

			status =  RAW_FALSE;
		}
		
	}
		
	if (status == RAW_TRUE) {

		*actual_flags_ptr = event_ptr->flags;
			
		/*does it need to clear the flags*/
		if (get_option & RAW_FLAGS_CLEAR_MASK) {
			
			event_ptr->flags &= ~requested_flags;
		}

		RAW_CRITICAL_EXIT();

		TRACE_EVENT_GET(raw_task_active, event_ptr);
		
		return RAW_SUCCESS;
	}
		
	/*Cann't get event, and return immediately if wait_option is  RAW_NO_WAIT*/
	if (wait_option == RAW_NO_WAIT) { 
		RAW_CRITICAL_EXIT();
		return RAW_NO_PEND_WAIT;
	}   

	/*system is locked so task can not be blocked just return immediately*/
	SYSTEM_LOCK_PROCESS();

	/*Remember the passed information*/
	raw_task_active->raw_suspend_option =  get_option;
	raw_task_active->raw_suspend_flags = requested_flags;
	raw_task_active->raw_additional_suspend_info = actual_flags_ptr;
		
	raw_pend_object((RAW_COMMON_BLOCK_OBJECT  *)event_ptr, raw_task_active, wait_option);
	RAW_CRITICAL_EXIT();

	TRACE_EVENT_GET_BLOCK(raw_task_active, event_ptr, wait_option);
	
	raw_sched(); 

	/*So the task is waked up, need know which reason cause wake up.*/
	error_status = block_state_post_process(raw_task_active, 0);

	return error_status;
		
}



RAW_U16 event_set(RAW_EVENT *event_ptr, RAW_U32 flags_to_set, RAW_U8 set_option)
{

	LIST *iter;
	LIST *event_head_ptr;
	LIST *iter_temp;
	RAW_TASK_OBJ *task_ptr;
	
	RAW_U8 status;
	RAW_U32 current_event_flags;
	
	RAW_SR_ALLOC();

	status = RAW_FALSE;
	
	RAW_CRITICAL_ENTER();

	if (event_ptr->common_block_obj.object_type != RAW_EVENT_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		
		return RAW_ERROR_OBJECT_TYPE;
	}

	event_head_ptr = &event_ptr->common_block_obj.block_list;

	/*if the set_option is AND_MASK, it just clear the flags and will return immediately!*/
	if (set_option & RAW_FLAGS_AND_MASK)  {

		event_ptr->flags &= flags_to_set;

		RAW_CRITICAL_EXIT();
		return RAW_SUCCESS;
	}
	
	/*if it is or mask then set the flag and continue.........*/
	else  {

		event_ptr->flags |= flags_to_set;    
	}

	current_event_flags = event_ptr->flags;
	iter = event_head_ptr->next;

	/*if list is not empty*/
 	while (iter != event_head_ptr) {

		task_ptr =  raw_list_entry(iter, RAW_TASK_OBJ, task_list);
		iter_temp =  iter->next;
		
		if (task_ptr->raw_suspend_option & RAW_FLAGS_AND_MASK)  {

			if ((current_event_flags  & task_ptr ->raw_suspend_flags) == task_ptr ->raw_suspend_flags) {
				status =  RAW_TRUE;
			}
			
			else {
				status =   RAW_FALSE;
			}
		}

		
		else {

			if (current_event_flags  &  task_ptr ->raw_suspend_flags) {
				
				status =  RAW_TRUE;
			}
			
			else {
				status =  RAW_FALSE;
			}
		}

		
		if (status == RAW_TRUE) {

			(*(RAW_U32 *)(task_ptr->raw_additional_suspend_info)) = current_event_flags;
			
			/*Ok the task condition is met, just wake this task*/
			raw_wake_object(task_ptr);

			/*does it need to clear the flags*/
			if (task_ptr->raw_suspend_option & RAW_FLAGS_CLEAR_MASK) {

				event_ptr->flags &= ~(task_ptr ->raw_suspend_flags);
			}

			TRACE_EVENT_WAKE(raw_task_active, task_ptr);

		}

		iter = iter_temp;

 	}

	RAW_CRITICAL_EXIT();

	raw_sched();
	
	return RAW_SUCCESS;

}

	 
/*
************************************************************************************************************************
*                                       Set an event
*
* Description: This service sets or clears event flags in an event flags group, depending
*						upon the specified set-option. All suspended threads whose event flags
*						request is now satisfied are resumed.
*
* Arguments  :event_ptr: is the address of event object
*                    -----
*                   flags_to_set: is the event flags to set or clear based
*                   upon the set option selected.
*
*						  -----
*			           set_option Specifies whether the event flags specified are
*						ANDed or ORed into the current event flags of
*						the group. The following are valid selections:
*						RAW_AND 
*					
*						RAW_OR 
*						Selecting RAW_AND specifies that the specified
*						event flags are ANDed into the current event
*						flags in the group andthis option will not wake any event and  is often used to
*						clear event flags in a group. Otherwise, if RAW_OR
*						is specified, the specified event flags are ORed
*						with the current event in the group and this option may wake event        
*				         
* Returns   RAW_SUCCESS : Set event success.
*				   RAW_OBJ_INVALIDATE_STATE:invalidate task state, possibly system error
*
* Note(s)    :if the set_option is AND_MASK, it just clear the flags and will return immediately!
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_event_set(RAW_EVENT *event_ptr, RAW_U32 flags_to_set, RAW_U8 set_option)
{

	#if (RAW_EVENT_FUNCTION_CHECK > 0)

	if (event_ptr == 0) {
		return RAW_NULL_OBJECT;
	}
	
	if ((set_option != RAW_AND) && (set_option != RAW_OR)) {
		return RAW_NO_THIS_OPTION;
	}
	
	#endif


	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
	
	if (raw_int_nesting) {
		
		return int_msg_post(RAW_TYPE_EVENT, event_ptr, 0, 0u, flags_to_set, set_option);
	}
	
	#endif

	return event_set(event_ptr, flags_to_set, set_option);		
	 
}



/*
************************************************************************************************************************
*                                       Delete an event
*
* Description: This service deletes the specified event flags group. All threads
*						suspended waiting for events from this group are resumed and given a
*						RAW_B_DEL return status.
*
* Arguments  :event_ptr is the address of event object
*                    -----
*                
*				         
*				         
* Returns   RAW_SUCCESS : delete event success.
*
* Note(s)    :RAW_STATE_UNKNOWN wrong task state, probally sysytem error.
*					 
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_EVENT_DELETE > 0)
RAW_U16 raw_event_delete(RAW_EVENT *event_ptr)
{
	LIST *block_list_head;

	RAW_SR_ALLOC();

	#if (RAW_EVENT_FUNCTION_CHECK > 0)

	if (event_ptr == 0) {
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}

	#endif

	RAW_CRITICAL_ENTER();

	if (event_ptr->common_block_obj.object_type != RAW_EVENT_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &event_ptr->common_block_obj.block_list;
	
	event_ptr->common_block_obj.object_type = 0u;
	/*All task blocked on this queue is waken up until list is empty*/
	while (!is_list_empty(block_list_head)) {
		
		delete_pend_obj(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list));	
	}    

	event_ptr->flags = 0u;

	RAW_CRITICAL_EXIT();

	TRACE_EVENT_DELETE(raw_task_active, event_ptr);

	raw_sched();  

	return RAW_SUCCESS;
}

#endif

#endif

