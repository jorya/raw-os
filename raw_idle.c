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


/* 	2012-9  Created by jorya_txj
  *	xxxxxx   please added here
  */

#include <raw_api.h>


#if (RAW_SYSTEM_CHECK > 0)
RAW_VOID raw_idle_task(void *p_arg)
{
	LIST *iter;
	RAW_TASK_OBJ						 *task_ptr;
	PORT_STACK  *task_stack;
	
	RAW_U32 free_stack;
	
	RAW_SR_ALLOC();
	p_arg = p_arg;                                          /* Make compiler happy ^_^ */
	
	iter = system_debug.task_head.next;
	
	while (1) {	
		
		free_stack = 0u;
		task_ptr = list_entry(iter,RAW_TASK_OBJ, stack_check_list);

		#if (RAW_CPU_STACK_DOWN > 0)
		
		task_stack = task_ptr->task_stack_base;  

		/*if no more freespace then break*/
		while (*task_stack++ == 0u) {                         
			free_stack++;

			/*if task free stack space is big than 12% ,then break because we have enougf stack space*/
			if (free_stack > (task_ptr->stack_size >> 3)) {
				break;
			}
				
		}
		
		#else 

		task_stack = (PORT_STACK *)(task_ptr->task_stack_base) + task_ptr->stack_size - 1;

		while (*task_stack-- == 0) {
			free_stack++;

			/*if task free stack space is big than 12% ,then break because we have enougf stack space*/
			if (free_stack > (task_ptr->stack_size >> 3)) {
				break;
			}
		}
		
		#endif
		
		TRACE_TASK_STACK_SPACE(task_ptr);
		
		RAW_CPU_DISABLE();
		/*if task is still on the stack check list*/
		if (task_ptr->task_state != RAW_DELETED) {

			/*if stack space is less than 12%*/
			if (free_stack < (task_ptr->stack_size >> 3)) {

				RAW_ASSERT(0);
			}

			iter = iter->next;

			/*if meet task head then skip it*/
			if (iter == (&(system_debug.task_head))) {
				iter = system_debug.task_head.next;
			}
				
		}

		/*if task is deleted, then move to next*/
		else {

			iter = system_debug.after_delete_list;
		}
		
		RAW_CPU_ENABLE();
	 		
	}
	
}

#else


RAW_VOID raw_idle_task (void *p_arg)
{
	p_arg = p_arg;                                          /* Make compiler happy ^_^ */
	raw_idle_coroutine_hook();
	
}

#endif




