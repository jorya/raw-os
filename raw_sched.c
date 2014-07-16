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


void raw_sched(void)
{
	RAW_SR_ALLOC();

	/*if it is in interrupt or system is locked, just return*/ 
	if (raw_int_nesting || raw_sched_lock) {              
		return;                                             
	}

	USER_CPU_INT_DISABLE();
	         
	get_ready_task(&raw_ready_queue);

	/*if highest task is currently task, then no need to do switch and just return*/
	if (high_ready_obj == raw_task_active) {                 
		USER_CPU_INT_ENABLE();                                     
		return;
	}

	TRACE_TASK_SWITCH(raw_task_active, high_ready_obj);

	CONTEXT_SWITCH(); 

	USER_CPU_INT_ENABLE();  

}


/*
************************************************************************************************************************
*                                       Init raw os
*
* Description: This function is called to init raw os.
*
* Arguments  :None
*                 -----
*                 
*
*				         
* Returns		RAW_U16:	 RAW_SUCCESS.
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_os_init(void)
{

	TRACE_INIT();
	
	raw_os_active = RAW_OS_STOPPED;
	run_queue_init(&raw_ready_queue);

	/*Init the tick heart system*/
	tick_list_init();

	/*Init the task debug head list*/
	list_init(&(raw_task_debug.task_head));

	#if (CONFIG_RAW_USER_HOOK > 0)
	raw_os_init_hook();
	#endif

	/*Start the first idle task*/
	raw_task_create(&raw_idle_obj, (RAW_U8  *)"idle_task",  0, 
									IDLE_PRIORITY, 0,  idle_stack, 
									IDLE_STACK_SIZE,  raw_idle_task, 1);
	
	#if (CONFIG_RAW_TIMER > 0)
	raw_timer_init();
	raw_mutex_create(&timer_mutex, (RAW_U8 *)"timer_mutex", RAW_MUTEX_INHERIT_POLICY, 0);
	#endif

	#if (CONFIG_RAW_TASK_0 > 0)
	raw_task_0_init();
	#endif

	#if (CONFIG_RAW_TICK_TASK > 0)
	tick_task_start();
	#endif

	#if (RAW_CONFIG_CPU_TASK > 0)
	cpu_task_start();
	#endif
	
	return RAW_SUCCESS;
}


/*
************************************************************************************************************************
*                                       Start raw os first task
*
* Description: This function is called to start raw os first task.
*
* Arguments  :None
*                 -----
*                 
*
*				         
* Returns		RAW_U16:	RAW_SYSTEM_ERROR.
*						
* Note(s)    This function shoud not returned!	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_os_start(void)
{

	if (raw_os_active == RAW_OS_STOPPED) {
		
		get_ready_task(&raw_ready_queue);
		
		/*done it here, so doesn't need do it in raw_start_first_task*/
		raw_task_active = high_ready_obj;
		raw_os_active = RAW_OS_RUNNING;
		
		raw_start_first_task(); 
	} 

	else {

		return RAW_OS_RUNNING;

	}

	return RAW_SYSTEM_ERROR;
	
}



#if (CONFIG_SCHED_FIFO_RR > 0)

void calculate_time_slice(RAW_U8 task_prio)
{
	RAW_TASK_OBJ   *task_ptr;
	LIST *head;

	RAW_SR_ALLOC();

	head = &raw_ready_queue.task_ready_list[task_prio];
	 
	RAW_CRITICAL_ENTER();
	
	/*if ready list is empty then just return because nothing is to be caculated*/                       
	if (is_list_empty(head)) {

		RAW_CRITICAL_EXIT();
		return;
	}

	/*Always look at the first task on the ready list*/
	task_ptr = raw_list_entry(head->next, RAW_TASK_OBJ, task_list);

	/*SCHED_FIFO does not has timeslice, just return*/
	if (task_ptr->sched_way == SCHED_FIFO) {
		
		RAW_CRITICAL_EXIT();
		return;
	}

	/*there is only one task on this ready list, so do not need to caculate time slice*/
	/*idle task must satisfy this condition*/
	if (head->next->next == head)  {
		
		RAW_CRITICAL_EXIT();
		return;
		
	}

	if (task_ptr->time_slice) {
		task_ptr->time_slice--;
	}

	/*if current active task has time_slice, just return*/
	if (task_ptr->time_slice) {               
		RAW_CRITICAL_EXIT();
		return;
	}

	/*Move current active task to the end of ready list for the same priority*/
	move_to_ready_list_end(&raw_ready_queue, task_ptr);

	/*restore the task time slice*/ 
	task_ptr->time_slice = task_ptr->time_total;  
	
	RAW_CRITICAL_EXIT();
}


#endif

