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

void run_queue_init(RAW_RUN_QUEUE *rq)
{
	RAW_U8 i;
   /*Inisialize the lowest priority to run queue*/
	rq->highest_priority = CONFIG_RAW_PRIO_MAX;
	 
	/*List init for run queue*/
	for (i = 0; i < CONFIG_RAW_PRIO_MAX; i++ ) {
		list_init(&rq->task_ready_list[i]);
	}

}

RAW_INLINE void add_to_priority_list(LIST *head, RAW_TASK_OBJ *task_ptr)
{
	RAW_U8 val;
	
	LIST *q, *list_start, *list_end;

	list_start = list_end = head;
	val = task_ptr->priority;
	
	/*Find the right position to insert*/ 
	for (q = list_start->next; q != list_end; q = q->next) {
		if (raw_list_entry(q, RAW_TASK_OBJ, task_list)->priority > val) {
			break;
		}
	}

	list_insert(q, &task_ptr->task_list);

}


RAW_INLINE void bit_clear(void *base, RAW_U8 offset)
{
	register RAW_U8 *cp, mask;
	
	cp = (RAW_U8 *)base;
	cp += offset >> 3;

	mask = _BIT_SET_N(offset);

	*cp &= (RAW_U8)(~mask);
}


RAW_INLINE void bit_set( void *base, RAW_U8 offset)
{
	register RAW_U8 *cp, mask;
	
	cp = (RAW_U8 *)base;
	cp += offset >> 3;

	mask = _BIT_SET_N(offset);

	*cp |= mask;
}


/*add task_ptr to the ready list head*/
void add_ready_list_head(RAW_RUN_QUEUE *rq, RAW_TASK_OBJ *task_ptr)
{
	RAW_U8 priority = task_ptr->priority;

	list_insert(rq->task_ready_list[priority].next, &task_ptr->task_list);
	bit_set(rq->task_bit_map, priority);

	/*update highest_priority if task has higher priority*/
	if (priority < rq->highest_priority) {
		rq->highest_priority = priority;
	}	

}



/*add task_ptr to the ready list end*/
void add_ready_list_end(RAW_RUN_QUEUE *rq, RAW_TASK_OBJ *task_ptr)
{

	RAW_U8	 priority = task_ptr->priority;

	list_insert(&rq->task_ready_list[priority], &task_ptr->task_list);
	bit_set(rq->task_bit_map, priority);

	/*update highest_priority if current task has higher priority*/
	if (priority < rq->highest_priority) {

		rq->highest_priority = priority;

	}	
	

}



void add_ready_list(RAW_RUN_QUEUE *rq, RAW_TASK_OBJ *task_ptr)
{
	/*if task priority is equal current task priority then add to the end*/
	if (task_ptr->priority == raw_task_active->priority) {
		add_ready_list_end(rq, task_ptr);
	}
	/*if not add to the list front*/
	else {
		add_ready_list_head(rq, task_ptr);
	}
	
}


void remove_ready_list(RAW_RUN_QUEUE *rq, RAW_TASK_OBJ *task_ptr)
{

	RAW_S32 	i;
	RAW_U8	 priority = task_ptr->priority;

	list_delete(&task_ptr->task_list);

	/*if the ready list is not empty, we do not need to update the highest priority*/
	if (!is_list_empty(&rq->task_ready_list[priority]) ) {
		return;
	}

	bit_clear(rq->task_bit_map, priority);

	/*If task priority not equal to the highest priority, then we do not need to update the highest priority*/
	/*This condition happens when a current high priority task to suspend a low priotity task*/
	
	if (priority != rq->highest_priority) {
		return;
	}

	/*Find the highest ready task*/
	i = bit_search_first_one(rq->task_bit_map, priority, CONFIG_RAW_PRIO_MAX - priority);
	
	/*Update the next highest priority task*/
	if (i >= 0) {
		rq->highest_priority = priority + i;
	} 

	else {
		
		RAW_ASSERT(0);
	}
	
}



void move_to_ready_list_end(RAW_RUN_QUEUE *rq, RAW_TASK_OBJ *task_ptr)
{

	LIST *head = &rq->task_ready_list[task_ptr->priority];
	/*delete it first than add to list end again*/
	list_delete(&task_ptr->task_list);
	list_insert(head, &task_ptr->task_list);
	
}


#if (CONFIG_RAW_TASK_0 > 0)

void get_ready_task(RAW_RUN_QUEUE *rq)
{
	LIST *node;
	RAW_U8 highest_pri;
	
	if (task_0_events) {

		high_ready_obj = &raw_task_0_obj;
		return;
	}

	highest_pri = rq->highest_priority;
	/*Highest priority task must be the first element on the list*/
	node = rq->task_ready_list[highest_pri].next;

	/*Get the highest priority task object*/
	high_ready_obj = raw_list_entry(node, RAW_TASK_OBJ, task_list);
	
}


#else

void get_ready_task(RAW_RUN_QUEUE *rq)
{
	LIST *node ;
	RAW_U8 highest_pri = rq->highest_priority;
	/*Highest priority task must be the first element on the list*/
	node = rq->task_ready_list[highest_pri].next;

	/*Get the highest priority task object*/
	high_ready_obj = raw_list_entry(node, RAW_TASK_OBJ, task_list);
	
}

#endif


static RAW_U16 pend_task_wake_up(RAW_TASK_OBJ *task_ptr)
{

	/*wake up task depend on the different state of task*/
	switch (task_ptr->task_state) {
		
		case RAW_PEND:
		case RAW_PEND_TIMEOUT:
			
			/*remove task on the block list because task is waken up*/
			list_delete(&task_ptr->task_list); 
			
			/*add to the ready list again*/    
			add_ready_list(&raw_ready_queue, task_ptr);
			
			task_ptr->task_state = RAW_RDY;
		         
			break;

		case RAW_PEND_SUSPENDED:
		case RAW_PEND_TIMEOUT_SUSPENDED:
                     
			/*remove task on the block list because task is waken up*/
			list_delete(&task_ptr->task_list);                   
			
			task_ptr->task_state = RAW_SUSPENDED;
		           
			break;

		default:
			
			RAW_ASSERT(0);
			
		
			
	
	}

	/*remove task on the tick list because task is waken up*/
	tick_list_remove(task_ptr); 

	task_ptr->block_status = RAW_B_OK;

	/*task is nothing blocked on so reset it to 0*/
	task_ptr->block_obj = 0;
	
	return RAW_SUCCESS;


}





RAW_U16 raw_wake_object(RAW_TASK_OBJ *task_ptr)
{
	return pend_task_wake_up(task_ptr);	
}



RAW_U16 wake_send_msg(RAW_TASK_OBJ *task_ptr, RAW_VOID *msg)
{

	task_ptr->msg = msg; 
	
	return pend_task_wake_up(task_ptr);																
}


RAW_U16 wake_send_msg_size(RAW_TASK_OBJ *task_ptr, RAW_VOID *msg, RAW_U32 msg_size)
{
	
	task_ptr->msg = msg; 
	task_ptr->msg_size = msg_size;
	
	return pend_task_wake_up(task_ptr);
												
}


RAW_U16 raw_pend_object(RAW_COMMON_BLOCK_OBJECT  *block_common_obj, RAW_TASK_OBJ *task_ptr, RAW_TICK_TYPE timeout)
{

	/*timeout 0 should not happen here, it has been processed before*/
	if (timeout == 0u) {
		RAW_ASSERT(0);
	}
		

	/*task need to remember which object is blocked on*/
	task_ptr->block_obj = block_common_obj;

	
	if (timeout == RAW_WAIT_FOREVER) {
		

		task_ptr->task_state = RAW_PEND;

	}
	/*task is blocked with timeout*/
	else {
		
		/*add to time sorted tick list */   
		tick_list_insert(task_ptr,timeout);

		task_ptr->task_state = RAW_PEND_TIMEOUT;

	}
	
	/*Remove from the ready list*/
	remove_ready_list(&raw_ready_queue, task_ptr);
	
	if (block_common_obj->block_way == RAW_BLOCKED_WAY_FIFO) {
		/*add to the end of blocked objet list*/
		list_insert(&block_common_obj->block_list, &task_ptr->task_list);

	}

	else {
		
		/*add to the priority sorted block list*/
		add_to_priority_list(&block_common_obj->block_list, task_ptr);
		
	}
	
	return RAW_SUCCESS;
}


RAW_U16 delete_pend_obj(RAW_TASK_OBJ *task_ptr)																	
{
	switch (task_ptr->task_state) {
		case RAW_PEND:
		case RAW_PEND_TIMEOUT:
		    
			/*remove task on the block list because task is waken up*/
			list_delete(&task_ptr->task_list);
           /*add to the ready list again*/        
			add_ready_list(&raw_ready_queue, task_ptr);
			task_ptr->task_state = RAW_RDY;
			           
			break;

		case RAW_PEND_SUSPENDED:
		case RAW_PEND_TIMEOUT_SUSPENDED:
                               
			/*remove task on the block list because task is waken up*/
			list_delete(&task_ptr->task_list);
		
			task_ptr->task_state = RAW_SUSPENDED;
			           
			break;

		default:
		
			RAW_ASSERT(0);
			
	}

	/*remove task on the tick list because task is waken up*/     
	tick_list_remove(task_ptr);
	task_ptr->block_status = RAW_B_DEL;  

	/*task is nothing blocked on so reset it to 0*/
	task_ptr->block_obj = 0;
	
	return RAW_SUCCESS;

}




void change_pend_list_priority(RAW_TASK_OBJ *task_ptr)
{

	RAW_COMMON_BLOCK_OBJECT *temp = task_ptr->block_obj;
	
	if (temp->block_way == RAW_BLOCKED_WAY_PRIO) {
		/*remove it first and add it again in priority sorted list*/
		list_delete(&task_ptr->task_list);
		add_to_priority_list(&task_ptr->block_obj->block_list, task_ptr);
	}

}

