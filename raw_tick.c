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

void tick_list_init(void)
{
	RAW_U16 i;

	for (i = 0; i < TICK_HEAD_ARRAY; i++ ) {
		list_init(&tick_head[i]);
	}

}
	
RAW_INLINE void tick_list_priority_insert(LIST *head, RAW_TASK_OBJ *task_ptr)
{
	RAW_TICK_TYPE val;
	
	LIST *q, *list_start, *list_end;
	RAW_TASK_OBJ  *task_iter_temp;

	list_start = list_end = head;
	val = task_ptr->tick_remain;
	

	for (q = list_start->next; q != list_end; q = q->next) {
		
		task_iter_temp = raw_list_entry(q, RAW_TASK_OBJ, tick_list);
		
		/*sorted by remain time*/
		
		if ((task_iter_temp->tick_match - raw_tick_count) > val) {
			break;
		}
	}

	list_insert(q, &task_ptr->tick_list);


}


void  tick_list_insert(RAW_TASK_OBJ *task_ptr, RAW_TICK_TYPE time)
                        
{
	LIST     *tick_head_ptr;

	RAW_U16   spoke;

	if (time) {
	                               
		task_ptr->tick_match = raw_tick_count + time;
		task_ptr->tick_remain = time;

		spoke   = (RAW_U16)(task_ptr->tick_match  &  (TICK_HEAD_ARRAY - 1) );
		tick_head_ptr = &tick_head[spoke];

		tick_list_priority_insert(tick_head_ptr, task_ptr);

		task_ptr->tick_head = tick_head_ptr;   

	}                
	
}



void tick_list_remove(RAW_TASK_OBJ  *task_ptr)
{
	LIST  *tick_head_ptr;
    
	tick_head_ptr = task_ptr->tick_head;

	if (tick_head_ptr) {
		list_delete(&task_ptr->tick_list);
		task_ptr->tick_head = 0;
		
	}


}


/*
************************************************************************************************************************
*                                    Update system tick time
*
* Description: This function is called to update system tick time.
*
* Arguments  :None
*                  
*                
*				         
*				         
* Returns   None
*				   
* Note(s) :This function is called by internal, users shoud not touch this function.
*
*             
************************************************************************************************************************
*/
void tick_list_update(void)
{
	
	LIST     *tick_head_ptr;
	RAW_TASK_OBJ            *p_tcb;
	LIST                            *iter;
	LIST                            *iter_temp;

	RAW_U16   spoke;

	RAW_SR_ALLOC();

	RAW_CRITICAL_ENTER();
	
	raw_tick_count++;                                                     
	spoke    = (RAW_U16)(raw_tick_count &  (TICK_HEAD_ARRAY - 1) );
	tick_head_ptr  = &tick_head[spoke];
	iter    = tick_head_ptr->next;
	
	while (RAW_TRUE) {

		/*search all the time list if possible*/
		if (iter != tick_head_ptr) {

			iter_temp =  iter->next;
			p_tcb =  raw_list_entry(iter, RAW_TASK_OBJ, tick_list);

			/*Since time list is sorted by remain time, so just campare  the absolute time*/
			if (raw_tick_count == p_tcb->tick_match) {
			
				switch (p_tcb->task_state) {
					case RAW_DLY:
						
						p_tcb->block_status = RAW_B_OK; 
						p_tcb->task_state = RAW_RDY;  
						tick_list_remove(p_tcb);
						add_ready_list(&raw_ready_queue, p_tcb);
						break; 

					case RAW_PEND_TIMEOUT:
						
						tick_list_remove(p_tcb);
						/*remove task on the block list because task is timeout*/
						list_delete(&p_tcb->task_list); 
						add_ready_list(&raw_ready_queue, p_tcb);
						p_tcb->block_status = RAW_B_TIMEOUT; 
						p_tcb->task_state = RAW_RDY; 
						
						#if (CONFIG_RAW_MUTEX > 0)
						mutex_state_change(p_tcb);
						#endif

						p_tcb->block_obj = 0;
						break;
						
					case RAW_PEND_TIMEOUT_SUSPENDED:

						tick_list_remove(p_tcb);
						/*remove task on the block list because task is timeout*/
						list_delete(&p_tcb->task_list); 
						p_tcb->block_status = RAW_B_TIMEOUT; 
						p_tcb->task_state = RAW_SUSPENDED;  
						
						#if (CONFIG_RAW_MUTEX > 0)
						mutex_state_change(p_tcb);
						#endif
					
						p_tcb->block_obj = 0;
						break;
					 
					case RAW_DLY_SUSPENDED:
										      
						p_tcb->task_state  =  RAW_SUSPENDED;
						p_tcb->block_status = RAW_B_OK; 
						tick_list_remove(p_tcb);                   
						break;

					default:
						
						RAW_ASSERT(0);
											
				}

				iter  = iter_temp;
			}

		/*if current task time out absolute time is not equal current system time, just break because timer list is sorted*/
			else {
			
				break;

			}

		}

		
		/*finish all the time list search */ 
		
		else {
			
			break;
		}
		
	}

	RAW_CRITICAL_EXIT();
}

#if (CONFIG_RAW_TICK_TASK > 0)

static void tick_task_process(void *para)
{
	RAW_U16 ret;
	
	while (1) {
		
		ret = raw_task_semaphore_get(RAW_WAIT_FOREVER);

		if (ret == RAW_SUCCESS) {
			if (raw_os_active == RAW_OS_RUNNING) {
				tick_list_update();
			}
		}
		
	}

}



void tick_task_start(void)
{

	/*Create tick task to caculate task sleep and timeout*/
	raw_task_create(&tick_task_obj, (RAW_U8  *)"tick_task_object",  0, 
	TICK_TASK_PRIORITY,  0, tick_task_stack, TICK_TASK_STACK_SIZE, tick_task_process, 1);

	raw_task_semaphore_create(&tick_task_obj, &tick_semaphore_obj, (RAW_U8 *)"tick_semaphore_obj", 0); 

}

#endif


