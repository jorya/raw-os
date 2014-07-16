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
*                                         Create a task
*
* Description: This function is called to create a task.
*
* Arguments  : task_obj      is a pointer to the  RAW_TASK_OBJ.
*               	------
*              	task_name  	is a string name assigned to a task
*			------
*			task_arg   is an argument passing to task
*			------
*			task_prio   is a priority to task, smalled priority is higher priority
*			 -------
*			time_slice  is run time slice tick to task, assign to 0 means it will accept default slice time
*			-------
*			task_stack_base is a low address of memory
*			------
*			stack_size is the number of stack elements of this task
*			------
*			task_entry is the entry of this task
*			 ------
*			auto_start is the flag to activate task:
*			         							RAW_AUTO_START	1
*				                                                RAW_DONT_START	0
*				         
* Returns    :		RAW_IDLE_EXIT   the idle priority should only be created once.
*        			 -----
*        			 RAW_OS_STOPPED  raw os has not been started yet
*        			 -----
*        			 RAW_SUCCESS  raw os return success.
*
* Note(s)    :
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_TASK_CREATE > 0)

RAW_U16 raw_task_create(RAW_TASK_OBJ  *task_obj, RAW_U8  *task_name,  RAW_VOID   *task_arg, 
                             RAW_U8  task_prio,  RAW_U32  time_slice,  PORT_STACK  *task_stack_base, 
                             RAW_U32 stack_size, RAW_TASK_ENTRY task_entry, RAW_U8 auto_start)
                                      
{
	PORT_STACK *p_stack;
	RAW_U32 i;
	
	RAW_SR_ALLOC();
	
	#if (RAW_TASK_FUNCTION_CHECK > 0)

	if (task_obj == 0) {
		return RAW_NULL_OBJECT;
	}
	
	if (task_prio >= CONFIG_RAW_PRIO_MAX) {
		return RAW_BYOND_MAX_PRIORITY;
	}
	
	if (task_stack_base == 0) {
		return RAW_NULL_POINTER;
	}

	if (task_entry == 0) {
		return RAW_NULL_POINTER;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}
	
	#endif

	RAW_CRITICAL_ENTER();
	
	 if (task_prio == IDLE_PRIORITY) {
	 	
	 	if (idle_task_exit) {
			
			RAW_CRITICAL_EXIT();
			return RAW_IDLE_EXIT;
				
	 	}
		
	 	idle_task_exit = 1u;
	}

	#if (CONFIG_RAW_TASK_0 > 0)
	
	 if (task_prio == 0) {
	 	
	 	if (task_0_exit) {
			
			RAW_CRITICAL_EXIT();
			return RAW_TASK_0_EXIT;
				
	 	}
		
	 	task_0_exit = 1u;
	}

	#endif
	

	RAW_CRITICAL_EXIT();
	
 	raw_memset(task_obj, 0, sizeof(RAW_TASK_OBJ));

	#if (CONFIG_SCHED_FIFO_RR > 0)
	
	if (time_slice) {
		task_obj->time_total        = time_slice;
		
	}
	
	else  {
		
		task_obj->time_total        = TIME_SLICE_DEFAULT;
	}

	task_obj->time_slice = task_obj->time_total;

	task_obj->sched_way = SCHED_RR;

	#endif
	
	if (auto_start) {
		task_obj->task_state = RAW_RDY;
	}
	
	else {
		task_obj->task_state = RAW_SUSPENDED;
		task_obj->suspend_count = 1u;
	}

	/*init all the stack element to 0*/
	task_obj->task_stack_base = task_stack_base;
	p_stack = task_stack_base;

	for (i = 0; i < stack_size; i++) {                           
		*p_stack++ = 0;                                            
	          
	}
		
	#if (CONFIG_RAW_USER_HOOK > 0)
	task_create_hook(task_obj);
	#endif
	
	task_obj->task_stack = port_stack_init(task_stack_base, stack_size, task_arg, task_entry);
	task_obj->task_name  = task_name; 
	task_obj->priority   = task_prio;
	task_obj->bpriority  = task_prio;
	task_obj->stack_size = stack_size;

	TRACE_TASK_CREATE(task_obj);
	
	RAW_CRITICAL_ENTER();
	
	list_insert(&(raw_task_debug.task_head), &task_obj->task_debug_list);

	if (auto_start) {
		add_ready_list_end(&raw_ready_queue, task_obj);
	}
	
	if (raw_os_active !=  RAW_OS_RUNNING) {                 /* Return if multitasking has not started                 */
		RAW_CRITICAL_EXIT();
		return RAW_OS_STOPPED;
	}

	RAW_CRITICAL_EXIT();

	if (auto_start) {
		raw_sched();
	}
	
	return RAW_SUCCESS;
          
}

#endif


/*
************************************************************************************************************************
*                                       Check Free elemenets of task stack
*
* Description: This function is called to   check free stack elemenets of task stack
*
* Arguments  :Free_stack is pointer to RAW_U32 and it will be filled later.It will be filled with free stack elements but not freebytes.
*                	 -----
*                
*				         
*				         
* Returns   RAW_SUCCESS:  raw os return success.
*
* Note(s)    :Stack space probably be used more than this API get, so increase about 20% stack space!
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_TASK_STACK_CHECK > 0)

RAW_U16 raw_task_stack_check(RAW_TASK_OBJ  *task_obj, RAW_U32 *free_stack) 
{
	PORT_STACK  *task_stack;
	RAW_U32 free_stk = 0;

	#if (RAW_TASK_FUNCTION_CHECK > 0)
	
	if (task_obj == 0) {
		return RAW_NULL_OBJECT;
	}

	if (free_stack == 0) {
		return RAW_NULL_POINTER;
	}

	#endif

	#if (RAW_CPU_STACK_DOWN > 0)
	
	task_stack = task_obj->task_stack_base;  
	
	while (*task_stack++ == 0) {                         
		free_stk++;
	}

	#else
	
	task_stack = (PORT_STACK *)(task_obj->task_stack_base) + task_obj->stack_size - 1;

	while (*task_stack-- == 0) {
		free_stk++;
	}

	#endif
	
	*free_stack = free_stk;

	return RAW_SUCCESS;
	 

}

#endif

/*
************************************************************************************************************************
*                                        disable the whole schedule
*
* Description: This function is called to disable the preempt of the raw os
*
* Arguments  : None
*                
*                
*				         
*				         
* Returns   RAW_SUCCESS  raw os return success.
*               RAW_NOT_CALLED_BY_ISR: can not call this API under ISR
* Note(s)    :
*
*             
************************************************************************************************************************
*/


RAW_U16 raw_disable_sche(void)
{
	RAW_SR_ALLOC();

	#if (RAW_TASK_FUNCTION_CHECK > 0)
	
	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}


	if (raw_sched_lock >= 250u)  {
		
		return RAW_SCHED_OVERFLOW;
		
	}
	
	#endif
	
	RAW_CPU_DISABLE();

	#if (RAW_SCHE_LOCK_MEASURE_CHECK > 0)

	sche_disable_measure_start();
	
	#endif
	
	raw_sched_lock++;
	RAW_CPU_ENABLE();
	
	return RAW_SUCCESS;
}



/*
************************************************************************************************************************
*                                       Enable  the whole schedule
*
* Description: This function is called to enable the preempt of the raw os
*
* Arguments  : NONE
*                    
*                    
*				         
*				         
* Returns   RAW_SUCCESS  raw os return success.
*               RAW_NOT_CALLED_BY_ISR: can not call this API under ISR
* Note(s)    :
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_enable_sche(void)
{

	RAW_SR_ALLOC();
	
	#if (RAW_TASK_FUNCTION_CHECK > 0)
	
	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}
		

	if (raw_sched_lock == 0u) {
		
		return RAW_SCHED_INVALID;
	}

	
	#endif

	#if (CONFIG_RAW_TASK_0 > 0)


	if (raw_sched_lock == 1u) {
		
		#if (RAW_SCHE_LOCK_MEASURE_CHECK > 0)
		
		sche_disable_measure_stop();
		
		#endif
		
		hybrid_int_process();							  
	}
	
	else {

		RAW_CPU_DISABLE();
		raw_sched_lock--;
		RAW_CPU_ENABLE();
		return RAW_SCHED_LOCKED;	
	}

	
	#else
	
	RAW_CPU_DISABLE();

	#if (RAW_SCHE_LOCK_MEASURE_CHECK > 0)

	sche_disable_measure_stop();

	#endif
	
	raw_sched_lock--;
	
	if (raw_sched_lock) {
		
		RAW_CPU_ENABLE();
		return RAW_SCHED_LOCKED;

	}
	
	RAW_CPU_ENABLE();

	#endif
	
	raw_sched ();
	
	return RAW_SUCCESS;
}


/*
************************************************************************************************************************
*                                        Task sleep
*
* Description: This function is called to cause the task to sleep
*
* Arguments  : dly if dly bigged than zero than this task will go to sleep as specified ticks.
*                    if dly equals zero than this fucntion implement the relinquish function , put the task back
*                    to the same priority list.
*                      
*                
*                
*				         
*				         
* Returns   RAW_SUCCESS:  raw os return success.
*              RAW_BLOCK_ABORT:Dly was aborted
*              RAW_SCHED_DISABLE:system is locked so task can not sleep 
* Note(s)    :
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_TASK_SLEEP > 0)

RAW_U16 raw_sleep(RAW_TICK_TYPE dly) 
{
	RAW_U16 error_status;
	
	RAW_SR_ALLOC();

	#if (RAW_TASK_FUNCTION_CHECK > 0)
	
	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
	}
	
	#endif		
		
	RAW_CRITICAL_ENTER();

	if (dly) {

		/*system is locked so task can not sleep just return immediately*/
		SYSTEM_LOCK_PROCESS();

		raw_task_active->task_state = RAW_DLY;

		tick_list_insert(raw_task_active, dly);
		           
		remove_ready_list(&raw_ready_queue, raw_task_active);
	}
	
	else {	
		/*make current task to the end of ready list*/
		 move_to_ready_list_end(&raw_ready_queue, raw_task_active);
	}

	RAW_CRITICAL_EXIT();

	raw_sched();   

	if (dly) {
		/*task is timeout after sleep*/
		error_status = block_state_post_process(raw_task_active, 0);
	}

	else {
		
		error_status = RAW_SUCCESS;

	}
	
	return error_status;
}


/*
************************************************************************************************************************
*                                        Task sleep with specified time.
*
* Description: This function is called to cause the task to sleep with specified time.
*
* Arguments  : hours: hours need to sleep, range from 0-999
*                     minutes:minutes need to sleep, range from 0-9999
*                     seconds:seconds need to sleep, range from 0-65535
*                     milli:mili seconds need to sleep, range from 0-4294967295.    			         
*				         
* Returns   RAW_SUCCESS:  raw os return success.
*              RAW_BLOCK_ABORT:Dly was aborted
*              RAW_SCHED_DISABLE:system is locked so task can not sleep
*              RAW_TIME_ZERO_SLEEP:zero ticks sleep is not allowed.
*
* Note(s)    :you may still need to check parameters to avoid  ticks overflow happen, if you sleep too long.
*                 this function is assumed RAW_TICKS_PER_SECOND is under 1000hz.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_time_sleep(RAW_U16 hours, RAW_U16 minutes, RAW_U16 seconds, RAW_U32 milli)
{

	RAW_TICK_TYPE tick_rate;
	RAW_TICK_TYPE ticks;

	#if (RAW_TASK_FUNCTION_CHECK > 0)

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}

	#endif	

	if (minutes > 9999u) {

		return RAW_INVALID_MINUTES;
	}
	
	if (hours > 999u) {
		
		return RAW_INVALID_HOURS;
	}

	tick_rate = RAW_TICKS_PER_SECOND;
	
	ticks = (hours * 3600u + minutes * 60u + seconds) * tick_rate
		+ (tick_rate * (milli + 500u / tick_rate)) / 1000u;

	if (ticks) {

		return raw_sleep(ticks);

	} 
	
	return RAW_TIME_ZERO_SLEEP;
		
}

#endif


/*
************************************************************************************************************************
*                                       Suspend  the specified task
*
* Description: This function is called to suspended the task specified by task_ptr
*
* Arguments  :task_ptr is the address of task object
*                   
*                
*				         
*				         
* Returns	:RAW_SCHED_LOCKED  loked task can not be suspended.
*
* Note(s)    	:RAW_STATE_UNKNOWN wrong task state, probally sysytem error.
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_TASK_SUSPEND > 0)

RAW_U16 raw_task_suspend(RAW_TASK_OBJ *task_ptr)
{
	
	#if (RAW_TASK_FUNCTION_CHECK > 0)
	
	if (task_ptr == 0) {
		return RAW_NULL_OBJECT;
	}
	
	#endif		

	if (task_ptr->priority == IDLE_PRIORITY) {
		return RAW_SUSPEND_TASK_NOT_ALLOWED;
	}

	#if (CONFIG_RAW_TASK_0 > 0)
	
	if (task_ptr->priority == 0) {
		return RAW_SUSPEND_TASK_NOT_ALLOWED;
	}
	
	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting) {
		
		return int_msg_post(RAW_TYPE_SUSPEND, task_ptr, 0, 0, 0, 0);
	}

	#endif
	
	return task_suspend(task_ptr);
	
}



RAW_U16 task_suspend(RAW_TASK_OBJ *task_ptr)
{
	RAW_SR_ALLOC();
	
	RAW_CRITICAL_ENTER();

	if (task_ptr == raw_task_active) {
		
		SYSTEM_LOCK_PROCESS();
	}

	switch (task_ptr->task_state) {
		case RAW_RDY:
			task_ptr->suspend_count = 1;
			task_ptr->task_state  =  RAW_SUSPENDED;
			remove_ready_list(&raw_ready_queue, task_ptr);
			break;

		case RAW_DLY:
			task_ptr->suspend_count = 1;
			task_ptr->task_state  = RAW_DLY_SUSPENDED;
			break;

		case RAW_PEND:
			task_ptr->suspend_count = 1;
			task_ptr->task_state  = RAW_PEND_SUSPENDED;
			break;
			

		case RAW_PEND_TIMEOUT:
			task_ptr->suspend_count = 1;
			task_ptr->task_state  = RAW_PEND_TIMEOUT_SUSPENDED;
			break;
			
		case RAW_SUSPENDED:
		case RAW_DLY_SUSPENDED:
		case RAW_PEND_SUSPENDED:
		case RAW_PEND_TIMEOUT_SUSPENDED:
			task_ptr->suspend_count++;
			if (task_ptr->suspend_count >= 250u) {

				RAW_CRITICAL_EXIT();
				return RAW_SUSPENDED_COUNT_OVERFLOWED;
			}

			break;
			
		default:
			
			RAW_CRITICAL_EXIT();
			
			return RAW_STATE_UNKNOWN;
	}

	RAW_CRITICAL_EXIT();

	TRACE_TASK_SUSPEND(raw_task_active, task_ptr);
	
	raw_sched();
	
	return RAW_SUCCESS;

}

#endif

/*
************************************************************************************************************************
*                                       Resume  the specified task
*
* Description: This function is called to resume the specified task.
*
* Arguments  :task_ptr is the address of task object
*                	 -----
*                
*				         
*				         
* Returns   RAW_SUCCESS  raw os return success.
*
* Note(s)    :
*
*             
************************************************************************************************************************
*/

#if (CONFIG_RAW_TASK_RESUME > 0)

RAW_U16 raw_task_resume(RAW_TASK_OBJ *task_ptr)
{
	#if (RAW_TASK_FUNCTION_CHECK > 0)
	
	if (task_ptr == 0) {
		return RAW_NULL_OBJECT;
	}
	
	#endif	

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting && raw_sched_lock) {
		return int_msg_post(RAW_TYPE_RESUME, task_ptr, 0, 0, 0, 0);
	}
	
	#endif
	
	return task_resume(task_ptr);
}

RAW_U16 task_resume(RAW_TASK_OBJ *task_ptr)
{

	RAW_SR_ALLOC();
	
	RAW_CRITICAL_ENTER();
		
	switch (task_ptr->task_state) {
		case RAW_RDY:
		case RAW_DLY:
		case RAW_PEND:
		case RAW_PEND_TIMEOUT:
			
			RAW_CRITICAL_EXIT();
			return HAS_NOT_SUSPENDED;
			

		case RAW_SUSPENDED:

			task_ptr->suspend_count--;

			if (task_ptr->suspend_count == 0) {

				/*Make task ready*/
				task_ptr->task_state = RAW_RDY;
				add_ready_list(&raw_ready_queue, task_ptr);
			}

			break;

		case RAW_DLY_SUSPENDED:
			
			task_ptr->suspend_count--;
			
			if (task_ptr->suspend_count == 0) {
						
				task_ptr->task_state = RAW_DLY;
			}
			
			break;

		case RAW_PEND_SUSPENDED:

			task_ptr->suspend_count--;

			if (task_ptr->suspend_count == 0) {
				task_ptr->task_state = RAW_PEND;
			}
			
			break;

		case RAW_PEND_TIMEOUT_SUSPENDED:
			
			task_ptr->suspend_count--;

			if (task_ptr->suspend_count == 0) {
				task_ptr->task_state = RAW_PEND_TIMEOUT;
			}
			
			break;
			
		default:
			
			RAW_CRITICAL_EXIT();
			
			return RAW_STATE_UNKNOWN;

	}

	RAW_CRITICAL_EXIT();

	TRACE_TASK_RESUME(raw_task_active, task_ptr);

	raw_sched();   

	return RAW_SUCCESS;

}

#endif



#if (CONFIG_RAW_TASK_PRIORITY_CHANGE > 0)

RAW_U16 change_internal_task_priority(RAW_TASK_OBJ *task_ptr, RAW_U8 new_priority)
{
	RAW_U8 old_pri = 0;
	old_pri = old_pri;

	switch (task_ptr->task_state) {
		case RAW_RDY:
			
			remove_ready_list(&raw_ready_queue, task_ptr);
			task_ptr->priority = new_priority;
			
			if (task_ptr == raw_task_active) {
				add_ready_list_head(&raw_ready_queue, task_ptr);
				
			}
			
			else {
			
				add_ready_list_end(&raw_ready_queue, task_ptr);
			}
	
			break;

		case RAW_DLY:                             /* Nothing to do except change the priority in the OS_TCB */
		case RAW_SUSPENDED:
		case RAW_DLY_SUSPENDED:
			
			task_ptr->priority = new_priority;                        /* Set new task priority*/
			
			break;

		case RAW_PEND:
		case RAW_PEND_TIMEOUT:
		case RAW_PEND_SUSPENDED:
		case RAW_PEND_TIMEOUT_SUSPENDED:
			old_pri = task_ptr->priority;
			task_ptr->priority = new_priority;  
			change_pend_list_priority(task_ptr);
			
			#if (CONFIG_RAW_MUTEX > 0)
			mtx_chg_pri(task_ptr, old_pri);
			#endif
			
			break;

		default:
			
			return RAW_STATE_UNKNOWN;
			
	}

	return RAW_SUCCESS;

}


/*
************************************************************************************************************************
*                                       Change the specified task priority
*
* Description: This function is called to change the specified task priority.
*
* Arguments  :task_ptr is the address of task object
*                   -----
*                  new_priority is the new priority assigned to.
*		     	-----
*                  old_priority will be filled with the orginal task priority if the task priority is changed success.
*
*				         
* Returns    	
*                	RAW_STATE_UNKNOWN: Probablly system errors happens
*			RAW_EXCEED_CEILING_PRIORITY: exceed mutex ceiling priority
*			RAW_SUCCESS: raw os return success
*			
* Note(s)    	
*l 
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_priority_change (RAW_TASK_OBJ *task_ptr, RAW_U8 new_priority, RAW_U8 *old_priority)
{
	RAW_U8 ret_pri = 0;
	RAW_U16 error;
	
	RAW_SR_ALLOC();

	ret_pri = ret_pri;

	#if (RAW_TASK_FUNCTION_CHECK > 0)

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
	}
	
	if (task_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (old_priority == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	#endif		

	/*Idle task is not allowed to change priority*/
	if (task_ptr->priority >= IDLE_PRIORITY) {
		
		return RAW_CHANGE_PRIORITY_NOT_ALLOWED;
	}

	#if (CONFIG_RAW_TASK_0 > 0)
	
	if (task_ptr->priority == 0) {
		
		return RAW_CHANGE_PRIORITY_NOT_ALLOWED;
	}

	if (new_priority == 0) {             

		return RAW_CHANGE_PRIORITY_NOT_ALLOWED;
	}

	#endif
	
	
   /*Not allowed change to idle priority*/
	if (new_priority == IDLE_PRIORITY) {             

		return RAW_CHANGE_PRIORITY_NOT_ALLOWED;
	}

	
	RAW_CRITICAL_ENTER();

	#if (CONFIG_RAW_MUTEX > 0)

	/*Limit the priority change by mutex at task priority change*/
	ret_pri = chg_pri_mutex(task_ptr, new_priority, &error);

	if (error != RAW_SUCCESS) {
		goto error_exit;
	}

	task_ptr->bpriority = new_priority;
	new_priority = ret_pri;
	
	#else
	
	task_ptr->bpriority = new_priority;
	#endif

	*old_priority = task_ptr->priority;

	error = change_internal_task_priority(task_ptr, new_priority);
	
	if (error != RAW_SUCCESS) {
		goto error_exit;
	}

	RAW_CRITICAL_EXIT();

	TRACE_TASK_PRIORITY_CHANGE(task_ptr, new_priority);
	
	raw_sched();  

 	return RAW_SUCCESS;
	
	error_exit:
	RAW_CRITICAL_EXIT();
	return error;
	
}

#endif


/*
************************************************************************************************************************
*                                       Delete the task
*
* Description: 	This function is called to delete the specified task and will cause reschdule
*
* Arguments  :	task_ptr is the address of task object
*                 		
*                  
*
*				         
* Returns    	:	RAW_STATE_UNKNOWN: Probablly system errors happens
*				RAW_SUCCESS: raw os return success
*
* Note(s)   		You can not delete a task which own a semphore or mutex.Highly recommended task to be deleted byself.
				Task state will change to  RAW_DELETED.
*				
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_TASK_DELETE > 0)
RAW_U16 raw_task_delete(RAW_TASK_OBJ *task_ptr)
{
	RAW_SR_ALLOC();

	#if (RAW_TASK_FUNCTION_CHECK > 0)

	if (task_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif

	if (task_ptr->priority == IDLE_PRIORITY) {
		
		return RAW_DELETE_TASK_NOT_ALLOWED;
	}

	#if (CONFIG_RAW_TASK_0 > 0)
	
	if (task_ptr->priority == 0) {
		
		return RAW_DELETE_TASK_NOT_ALLOWED;
	}

	#endif
	

	RAW_CRITICAL_ENTER();

	if (task_ptr == raw_task_active) {
		
		SYSTEM_LOCK_PROCESS();
	}

	#if (CONFIG_RAW_MUTEX > 0)
	raw_task_free_mutex(task_ptr);
	#endif
	
	switch (task_ptr->task_state) {
		case RAW_RDY:
			remove_ready_list(&raw_ready_queue, task_ptr);
			break;

		case RAW_SUSPENDED:
			break;

		case RAW_DLY:                            
		case RAW_DLY_SUSPENDED:
			tick_list_remove(task_ptr);
			break;

		case RAW_PEND:
		case RAW_PEND_SUSPENDED:
		case RAW_PEND_TIMEOUT:
		case RAW_PEND_TIMEOUT_SUSPENDED:
			tick_list_remove(task_ptr);
			list_delete(&task_ptr->task_list);
			task_ptr->task_state = RAW_DELETED;
			
			#if (CONFIG_RAW_MUTEX > 0)
			mutex_state_change(task_ptr);
			#endif
			
			break;

		default:
			
			RAW_CRITICAL_EXIT();
			
			return  RAW_STATE_UNKNOWN;
	}  

	task_ptr->task_state = RAW_DELETED;   
	
	list_delete(&task_ptr->task_debug_list);
	
	RAW_CRITICAL_EXIT();

	TRACE_TASK_DELETE(task_ptr);

	#if (CONFIG_RAW_USER_HOOK > 0)
	raw_task_delete_hook(task_ptr);
	#endif

	raw_sched();

	return RAW_SUCCESS;
}

#endif



/*
************************************************************************************************************************
*                                       Set the task user data pointer
*
* Description: This function is called to set the task user data pointer
*
* Arguments  :task_ptr is the address of task object
*                   -----
*                   user_point is the n user data pointer
*                   ------
*                   point_position is the position of the user_data_pointer array, it starts from 0!
*
* Returns   	:  point_position starts from 0.
* 
*
*             
************************************************************************************************************************
*/
#if (CONFIG_USER_DATA_POINTER > 0)

RAW_VOID raw_set_task_user_point(RAW_TASK_OBJ *task_ptr, RAW_VOID *user_point, RAW_U32 point_position)
{
	RAW_SR_ALLOC();

	RAW_CPU_DISABLE();
	task_ptr->user_data_pointer[point_position] = user_point;
	RAW_CPU_ENABLE();
}


RAW_VOID *raw_get_task_user_point(RAW_TASK_OBJ *task_ptr, RAW_U32 point_position)
{
	return task_ptr->user_data_pointer[point_position];	
}


#endif


/*
************************************************************************************************************************
*                                       Change the specified task time slice
*
* Description: This function is called to change the specified task time slice
*
* Arguments  :task_ptr is the address of task object
*                   -----
*                   new_time_slice is the new time slice assigned to task
*
*				            
* Returns    RAW_SUCCESS: raw os return success
* Note(s)    If new_time_slice is 0 then TIME_SLICE_DEFAULT will be the default time slice value
*
*             
************************************************************************************************************************
*/
#if (CONFIG_SCHED_FIFO_RR > 0)
RAW_U16 raw_task_time_slice_change(RAW_TASK_OBJ *task_ptr, RAW_U32 new_time_slice)
{
	RAW_SR_ALLOC();
	
	#if (RAW_TASK_FUNCTION_CHECK > 0)

	if (task_ptr == 0) {
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}
	
	#endif

	RAW_CRITICAL_ENTER();


	if (new_time_slice) {
		
		/*assign the new time slice*/
		task_ptr->time_total = new_time_slice;
	
	} 

	else {

		/*assign the default time slice*/
		task_ptr->time_total = TIME_SLICE_DEFAULT;
		
	}

	task_ptr->time_slice = task_ptr->time_total;
	
	RAW_CRITICAL_EXIT();
	
	return RAW_SUCCESS;
}


/*
************************************************************************************************************************
*                                       Change the specified task sched way
*
* Description: This function is called to  change the specified task sched way
*
* Arguments  :task_ptr is the address of task object
*                 -----
*                   policy is the sched method assigned to task, sched method can be followings:
*			SCHED_FIFO or SCHED_RR
*		      -----	         
* Returns     RAW_SUCCESS: raw os return success
*                 RAW_INVALID_SCHED_WAY: No this sched method
* Note(s)  
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_set_sched_way(RAW_TASK_OBJ *task_ptr, RAW_U8 policy)
{
	RAW_SR_ALLOC();
	
	#if (RAW_TASK_FUNCTION_CHECK > 0)

	if (task_ptr == 0) {
		return RAW_NULL_OBJECT;
	}


	if ((policy != SCHED_FIFO) && (policy != SCHED_RR)) {

		return RAW_INVALID_SCHED_WAY;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}
	
	#endif


	RAW_CRITICAL_ENTER();
	task_ptr->sched_way =  policy;
	RAW_CRITICAL_EXIT();

	return RAW_SUCCESS;
	
}



/*
************************************************************************************************************************
*                                       Change the specified task sched way
*
* Description:  This function is called to  change the specified task sched way
*
* Arguments  :task_ptr is the address of task object
*                	 -----
*                   policy is the sched method of the  task, and will be filled in later.
*		    	       
* Returns    	:RAW_SUCCESS: raw os return success
*               
* Note(s)  
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_get_sched_way(RAW_TASK_OBJ *task_ptr, RAW_U8 *policy_ptr)
{
	RAW_SR_ALLOC();
	
	#if (RAW_TASK_FUNCTION_CHECK > 0)

	if (task_ptr == 0) {
		return RAW_NULL_OBJECT;
	}


	if (policy_ptr == 0) {

		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}
	
	#endif


	RAW_CRITICAL_ENTER();
	*policy_ptr = task_ptr->sched_way;
	RAW_CRITICAL_EXIT();

	return RAW_SUCCESS;
	
}

#endif


/*
************************************************************************************************************************
*                                       Abort any dly and sunspension or and pend 
*
* Description: This function is called to abort the specified task.
*
* Arguments  :task_ptr is the address of task object
*               
*                 
*
*				         
* Returns	RAW_STATE_UNKNOWN: Probablly system errors happens
*			RAW_SUCCESS: raw os return success
*			
* Note(s)    	Any aborted task will return a RAW_BLOCK_ABORT
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_TASK_WAIT_ABORT > 0)
RAW_U16 raw_task_wait_abort(RAW_TASK_OBJ *task_ptr)
{
	RAW_SR_ALLOC();

	#if (RAW_TASK_FUNCTION_CHECK > 0)

	if (task_ptr == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();
	
	switch (task_ptr->task_state) {
		case RAW_RDY:
			break;
			
		case RAW_SUSPENDED:
			/*change to ready state*/
			task_ptr->task_state = RAW_RDY;
			add_ready_list(&raw_ready_queue, task_ptr);   
			break;

		case RAW_DLY:    
		case RAW_DLY_SUSPENDED:
			/*change to ready state*/
			tick_list_remove(task_ptr);
			add_ready_list(&raw_ready_queue, task_ptr);
			task_ptr->task_state = RAW_RDY;
			task_ptr->block_status = RAW_B_ABORT;   
			break;


		case RAW_PEND_SUSPENDED:
		case RAW_PEND_TIMEOUT_SUSPENDED:
		case RAW_PEND:
		case RAW_PEND_TIMEOUT:
			
			/*remove task on the tick list because task is waken up*/
			tick_list_remove(task_ptr);  
			/*remove task on the block list because task is waken up*/
			list_delete(&task_ptr->task_list);   
          	/*add to the ready list again*/    
			add_ready_list(&raw_ready_queue, task_ptr);
			task_ptr->task_state = RAW_RDY;
			task_ptr->block_status = RAW_B_ABORT;
			
			#if (CONFIG_RAW_MUTEX > 0)
			mutex_state_change(task_ptr);
			#endif

			task_ptr->block_obj = 0;
			
			break;
			
		default:
			
			RAW_CRITICAL_EXIT();
			
			return  RAW_STATE_UNKNOWN;
	}
	
	RAW_CRITICAL_EXIT();

	#if (CONFIG_RAW_USER_HOOK > 0)
	raw_task_abort_hook(task_ptr);
	#endif
	
	TRACE_TASK_WAIT_ABORT(task_ptr);
	
	raw_sched(); 
	
	return RAW_SUCCESS;
}

#endif

/*
************************************************************************************************************************
*                                       Get the current active task object
*
* Description: This function is called to get the current task object.
*
* Arguments  :None
*                 
*                 
*
*				         
* Returns	current active task object.
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_TASK_OBJ *raw_task_identify(void)
{

	return raw_task_active;

}


/*
************************************************************************************************************************
*                                     Debug blocked task on semphore, event, mutex,queue, block, byte memory
*
* Description: This function is called to debug blocked task on  semphore, event, mutex,queue, block, byte memory
*
* Arguments  :object_head can be and of semphore, event, mutex,queue.
*                 	-----
*                	debug_function would be called for each blocked task on on semphore, event, mutex,queue.
*		   	-----
*			opt decide whether to wake up task or not.
*                 	if opt > 0 then wake up all the blocked task.
*			if opt = 0, just do nothing
*				         
* Returns	
						
* Note(s)   	If no task blocked on these object then debug_function will not be called. 	
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_DEBUG > 0)

RAW_U16 raw_iter_block_task(LIST *object_head, RAW_VOID  (*debug_function)(RAW_TASK_OBJ *arg), RAW_U8 opt)
{
	LIST *iter;
	LIST *iter_temp;

	RAW_SR_ALLOC();

	#if (RAW_TASK_FUNCTION_CHECK > 0)

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();
	iter = object_head->next;
	
	/*do it until list pointer is back to the original position*/ 
	while (iter != object_head) {

		iter_temp  = iter->next;
		
		if (debug_function) {
			debug_function(raw_list_entry(iter, RAW_TASK_OBJ, task_list));
		}

		if (opt) {
			raw_wake_object(raw_list_entry(object_head->next, RAW_TASK_OBJ, task_list));
		}
		/*move to list next*/
		iter = iter_temp;
	}

	RAW_CRITICAL_EXIT(); 

	raw_sched();

	return RAW_SUCCESS;
	 
}




/*
************************************************************************************************************************
*                                     Get raw os data and bss data space
* Description: This function is called to get raw os data and bss data space
*
* Arguments  :None
*                			         
* Returns	raw os data plus bss data space
*						
* Note(s)  
*
*             
************************************************************************************************************************
*/
RAW_U32 raw_get_system_global_space(void)
{

	RAW_U32 data_space;
	
	data_space =  sizeof(raw_os_active) + sizeof(idle_task_exit) + sizeof(raw_ready_queue)
		                 +sizeof(raw_sched_lock) +sizeof(raw_int_nesting) + sizeof(high_ready_obj)
		                 + sizeof(raw_task_active) + sizeof(raw_idle_obj) + sizeof(idle_stack);

	
	#if (CONFIG_RAW_TIMER > 0)

	data_space += sizeof(timer_head) + sizeof(raw_timer_count) + sizeof(raw_timer_ctrl)
				+ sizeof(raw_timer_obj) + sizeof(timer_task_stack) + sizeof(timer_sem);
				
	#endif


	#if (CONFIG_RAW_TICK_TASK > 0)

	data_space += sizeof(tick_task_obj) + sizeof(tick_task_stack) + sizeof(tick_semaphore_obj);

	#endif

	data_space += sizeof(raw_task_debug);

	#if (CONFIG_RAW_MUTEX > 0)
	
	data_space += sizeof(mutex_recursion_levels) + sizeof(mutex_recursion_max_levels);

	#endif
	
	#if (CONFIG_RAW_TASK_0 > 0)
	
	data_space += sizeof(task_0_event_end) + sizeof(task_0_event_head)
				+ sizeof(task_0_events) + sizeof(peak_events)
				+ sizeof(task_0_events_queue)
				+ sizeof(raw_task_0_obj) + sizeof(task_0_stack) + sizeof(task_0_exit);


	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	data_space += sizeof(object_int_msg) + sizeof(free_object_int_msg) + sizeof(msg_event_handler)
				+ sizeof(int_msg_full);
				
	#endif
	
	#endif

	#if (CONFIG_RAW_IDLE_EVENT > 0)
	
	data_space += sizeof(STM_GLOBAL_EVENT) + sizeof(raw_idle_rdy_grp) + sizeof(raw_rdy_tbl)
				+ sizeof(raw_idle_map_table) + sizeof(raw_idle_tick_head);

	#endif
	
	
	return data_space;
	
}

#endif

