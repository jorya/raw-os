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

#if (CONFIG_RAW_TIMER > 0)

static void timer_list_init(void)
{
	RAW_U16 i;

	for (i = 0; i < TIMER_HEAD_NUMBERS; i++ ) {
		
		list_init(&timer_head[i]);
	}

}


	
static void timer_list_priority_insert(LIST   *head, RAW_TIMER *timer_ptr)
{
	RAW_TICK_TYPE val;
	
	LIST *q, *list_start, *list_end;
	
	RAW_TIMER  *task_iter_temp;

	list_start = list_end = head;
	val = timer_ptr->remain;
	
	for (q = list_start->next; q != list_end; q = q->next) {

		task_iter_temp = raw_list_entry(q, RAW_TIMER, timer_list);

		/*sorted by remain time*/
		
		if ((task_iter_temp->match - raw_timer_count) > val) {
			break;
		}
	}

	list_insert(q, &timer_ptr->timer_list);

}


static void timer_list_remove(RAW_TIMER *timer_ptr)
{
	LIST  *tick_head_ptr;
    
	tick_head_ptr = timer_ptr->to_head;

	if (tick_head_ptr) {
		
		list_delete(&timer_ptr->timer_list);
		timer_ptr->to_head = 0;
		
	}

}


/*
************************************************************************************************************************
*                                       Create a soft timer
*
* Description: This function is called to create a timer
*
* Arguments  :timer_ptr is the address of this timer object
*                    -----
*                	name_ptr is the name of this timer
*			 -----
*			expiration_function is the call back function when time expired
*			-----
*			initial_ticksis the first time to call the expiration_function
*                   -----
*			reschedule_ticks is the period time after firstcall the expiration_function
*			-----
*			auto_activate means whether this timer start automatically.
*				         
* Returns  	 RAW_SUCCESS  means timer create success.
*
* Note(s)    :if reschedule_ticks equals 0, the timer call back function run once.
*					 if auto_activate equals 0, you have to call raw_timer_activate later.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_timer_create(RAW_TIMER *timer_ptr, RAW_U8  *name_ptr, 
            RAW_U16  (*expiration_function)(RAW_VOID *expiration_input), RAW_VOID *expiration_input,
          RAW_TICK_TYPE initial_ticks, RAW_TICK_TYPE reschedule_ticks, RAW_U8 auto_activate)

{
	
	#if (RAW_TIMER_FUNCTION_CHECK > 0)
	
	if (timer_ptr == 0) {
		return RAW_NULL_OBJECT;
	}
	
	if (expiration_function == 0) {
		return RAW_NULL_POINTER;
	}

	if (initial_ticks == 0) {
		return RAW_TIMER_INVALID_TICKS;

	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;	
	}
	
	#endif
	
	timer_ptr->name = name_ptr;
	timer_ptr->raw_timeout_function = expiration_function;
	timer_ptr->raw_timeout_param = expiration_input;
	timer_ptr->init_count = initial_ticks;
	timer_ptr->reschedule_ticks = reschedule_ticks;
	timer_ptr->remain = 0u;
	timer_ptr->match = 0u;
	timer_ptr->timer_state = TIMER_DEACTIVE;
	timer_ptr->to_head = 0;
	
	list_init(&timer_ptr->timer_list);

	timer_ptr->object_type = RAW_TIMER_OBJ_TYPE;
	
	if (auto_activate) {
		
		 raw_timer_activate(timer_ptr, expiration_input);
	}

	return RAW_SUCCESS;
}






/*
************************************************************************************************************************
*                                       Activate  the specified timer
*
* Description: This function is called to activate  the specified timer
*
* Arguments  :timer_ptr is the address of this timer object
*                    -----
*                
*				         
*				         
* Returns   RAW_TIMER_STATE_INVALID:  means timer is already active or timer is deleted..
*				   RAW_SUCCESS: means timer activate success
* Note(s)    :RAW_STATE_UNKNOWN wrong task state, probally sysytem error.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_timer_activate(RAW_TIMER *timer_ptr, RAW_VOID *expiration_input)
{
	RAW_U16 position;
	RAW_U16 mutex_ret;
	
	#if (RAW_TIMER_FUNCTION_CHECK > 0)
	
	if (timer_ptr == 0) {
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
		
	}

	
	#endif

	if (timer_ptr->object_type != RAW_TIMER_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}
	
	/*Timer state TIMER_ACTIVE TIMER_DELETED is not allowed to delete*/
	if (timer_ptr->timer_state == TIMER_ACTIVE) {
		
		return RAW_TIMER_STATE_INVALID;	
	}

	if (timer_ptr->timer_state == TIMER_DELETED) {
		
		return RAW_TIMER_STATE_INVALID;
	}

	timer_ptr->raw_timeout_param = expiration_input;
	
	mutex_ret = raw_mutex_get(&timer_mutex, RAW_WAIT_FOREVER);
	RAW_ASSERT(mutex_ret == RAW_SUCCESS);
		
	timer_ptr->match  = raw_timer_count + timer_ptr->init_count;
	position   = (RAW_U16)(timer_ptr->match & (TIMER_HEAD_NUMBERS - 1) );
	/*Sort by remain time*/
	timer_ptr->remain = timer_ptr->init_count;
	/*Used by timer delete*/
	timer_ptr->to_head = &timer_head[position];
	
	timer_list_priority_insert(&timer_head[position], timer_ptr);
	timer_ptr->timer_state = TIMER_ACTIVE;
	raw_mutex_put(&timer_mutex);

	return RAW_SUCCESS;
}



/*
************************************************************************************************************************
*                                      Change  the specified timer parameter
*
* Description: This function is called to change the timer para specified by timer_ptr
*
* Arguments  :timer_ptr is the address of this timer object
*                    -----
*                
*				         
*				         
* Returns   RAW_TIMER_STATE_INVALID:  means timer state is not DEACTIVE or timer is deleted.
*				   RAW_SUCCESS: means timer change success.
* Note(s)    :Youm must call  raw_timer_deactivate before call this function
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_TIMER_CHANGE > 0)
RAW_U16 raw_timer_change(RAW_TIMER *timer_ptr, RAW_TICK_TYPE initial_ticks, RAW_TICK_TYPE reschedule_ticks)
{
	RAW_U16 mutex_ret;
	
	#if (RAW_TIMER_FUNCTION_CHECK > 0)
	
	if (timer_ptr == 0) {
		return RAW_NULL_OBJECT;
	}


	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif

	if (timer_ptr->object_type != RAW_TIMER_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}
	
	
	/*Only timer state TIMER_DEACTIVE is  allowed here*/	
	if (timer_ptr->timer_state != TIMER_DEACTIVE) {
		return RAW_TIMER_STATE_INVALID;
	}
	
	if (timer_ptr->timer_state == TIMER_DELETED) {
		return RAW_TIMER_STATE_INVALID;
	}
	
	mutex_ret = raw_mutex_get(&timer_mutex, RAW_WAIT_FOREVER);
	RAW_ASSERT(mutex_ret == RAW_SUCCESS);
	
	timer_ptr->init_count = initial_ticks;
	timer_ptr->reschedule_ticks = reschedule_ticks;
	raw_mutex_put(&timer_mutex);
	
	return RAW_SUCCESS;
}
#endif


/*
************************************************************************************************************************
*                                     Deactivate  this timer
*
* Description: This function is called to  Deactivate the timer by timer_ptr
*
* Arguments  :timer_ptr is the address of this timer object
*                    -----
*                
*				         
*				         
* Returns   RAW_TIMER_STATE_INVALID:  means timer state is already  DEACTIVE or timer is deleted.
*				   RAW_SUCCESS: means timer change success.
* Note(s)    :
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_TIMER_DEACTIVATE > 0)
RAW_U16 raw_timer_deactivate(RAW_TIMER *timer_ptr)
{
	RAW_U16 mutex_ret;
	
	#if (RAW_TIMER_FUNCTION_CHECK > 0)
	
	if (timer_ptr == 0) {
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif

	if (timer_ptr->object_type != RAW_TIMER_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}
	
	/*Timer state TIMER_DEACTIVE  TIMER_DELETED is not allowed to delete*/
	if (timer_ptr->timer_state == TIMER_DEACTIVE) {
		return RAW_TIMER_STATE_INVALID;
	}
	
	if (timer_ptr->timer_state == TIMER_DELETED) {
		return RAW_TIMER_STATE_INVALID;

	}
	
	mutex_ret = raw_mutex_get(&timer_mutex, RAW_WAIT_FOREVER);
	RAW_ASSERT(mutex_ret == RAW_SUCCESS);
	
	timer_list_remove(timer_ptr);
	timer_ptr->timer_state = TIMER_DEACTIVE;
	raw_mutex_put(&timer_mutex);
	
	return RAW_SUCCESS;

}
#endif


/*
************************************************************************************************************************
*                                    Delete  this timer
*
* Description: This function is called to  Deactivate the timer by timer_ptr
*
* Arguments  :timer_ptr is the address of this timer object
*                    -----
*                
*				         
*				         
* Returns   RAW_TIMER_STATE_INVALID:  means timer state is already deleted.
*				   RAW_SUCCESS: means timer delete success.
* Note(s)    :timer object is reseted.
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_TIMER_DELETE > 0)
RAW_U16 raw_timer_delete(RAW_TIMER *timer_ptr)
{
	RAW_U16 mutex_ret;
	
	#if (RAW_TIMER_FUNCTION_CHECK > 0)
	
	if (timer_ptr == 0) {
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;	
	}
	
	#endif

	if (timer_ptr->object_type != RAW_TIMER_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}
	
	if (timer_ptr->timer_state == TIMER_DELETED) {
		
		return RAW_TIMER_STATE_INVALID;
		
	}
	
	mutex_ret = raw_mutex_get(&timer_mutex, RAW_WAIT_FOREVER);
	RAW_ASSERT(mutex_ret == RAW_SUCCESS);
	
	timer_ptr->object_type = 0u;
	timer_list_remove(timer_ptr);
	timer_ptr->timer_state = TIMER_DELETED;
	raw_mutex_put(&timer_mutex);
   
	return RAW_SUCCESS;

}
#endif


/*
************************************************************************************************************************
*                                    Timer task 
*
* Description: This function is called to  start a timer task.
*
* Arguments  :pa is the parameters to task.
*                    -----
*                
*				         
*				         
* Returns   
*				   
* Note(s) :This function is called by internal, users shoud not touch this function.
*
*             
************************************************************************************************************************
*/
void timer_task(void *pa) 
{
	RAW_U16 							position;
	LIST 								*timer_head_ptr;
	LIST 								*iter;
	LIST 								*iter_temp;
	RAW_TIMER							*timer_ptr;
	RAW_U16                              mutex_ret;
	RAW_U16                              callback_ret;
	
	/*reset the timer_sem count since it may not be 0 at this point, make it start here*/
	raw_semaphore_set(&timer_sem, 0);
	pa = pa;
	
	while (1) {
		
		/*timer task will be blocked after call this function*/
		raw_semaphore_get(&timer_sem, RAW_WAIT_FOREVER);

		mutex_ret = raw_mutex_get(&timer_mutex, RAW_WAIT_FOREVER);
		RAW_ASSERT(mutex_ret == RAW_SUCCESS);

		/*calculate which  timer_head*/
		raw_timer_count++;                                          
		position = (RAW_U16)(raw_timer_count & (TIMER_HEAD_NUMBERS - 1) );
		timer_head_ptr  = &timer_head[position];

		iter = timer_head_ptr->next;
		
		while (RAW_TRUE) {

			/*if timer exits*/	
			if (iter != timer_head_ptr) {

				/*Must use iter_temp because iter may be remove later.*/
				iter_temp = iter->next;
				timer_ptr =  raw_list_entry(iter, RAW_TIMER, timer_list);

				/*if timeout*/
				if (raw_timer_count == timer_ptr->match) {  

					/*remove from timer list*/
					timer_list_remove(timer_ptr);

					/*if timer is reschedulable*/			
					if (timer_ptr->reschedule_ticks) {

						/*Sort by remain time*/
						timer_ptr->remain = timer_ptr->reschedule_ticks;

						timer_ptr->match  = raw_timer_count + timer_ptr->remain;
						position   = (RAW_U16)(timer_ptr->match & (TIMER_HEAD_NUMBERS - 1));
						timer_ptr->to_head = &timer_head[position];
						timer_list_priority_insert(&timer_head[position], timer_ptr);
					          
					} 

					else {

						timer_ptr->timer_state = TIMER_DEACTIVE;

					}

					/*Any way both condition need to call registered timer function*/
					/*the registered timer function should not touch any timer related API,otherwise system will be crashed*/
					if (timer_ptr->raw_timeout_function) {

						callback_ret = timer_ptr->raw_timeout_function(timer_ptr->raw_timeout_param);
						if ((callback_ret == TIMER_CALLBACK_STOP) && (timer_ptr->timer_state != TIMER_DEACTIVE)) {
							/*remove from timer list*/
							timer_list_remove(timer_ptr);
							timer_ptr->timer_state = TIMER_DEACTIVE;
						}
					         
					}

					iter  = iter_temp; 

				} 

				else { 

					break;	
				}

			}

			/*exit because timer is not exit*/		
			else {

				break;
			}

		}

		raw_mutex_put(&timer_mutex);

	}


}



/*
************************************************************************************************************************
*                                    Called by systen timer interrupt
*
* Description: This function is called to  start a timer task.
*
* Arguments  :None
*                   
*                
*				         
*				         
* Returns   
*				   
* Note(s) :This function is called by internal, users shoud not touch this function.
*
*             
************************************************************************************************************************
*/
void call_timer_task(void)
{
	raw_timer_ctrl--;
	
	if (raw_timer_ctrl == 0u) {
		
		/*reload timer frequency ctrl.*/
		raw_timer_ctrl = RAW_TIMER_RATE;
		/*Release a semphore to timer task*/
		raw_semaphore_put(&timer_sem);
	}
}


void raw_timer_init(void)
{
	raw_timer_ctrl = RAW_TIMER_RATE;
	
	timer_list_init();
	
	/*Create a timer task to handle soft timer*/
	raw_task_create(&raw_timer_obj, (RAW_U8  *)"timer_object",  0, 
	                  TIMER_TASK_PRIORITY,  0,   timer_task_stack, 
	                	TIMER_STACK_SIZE, timer_task, 1);

	/*create a semaphore for timer task*/
	raw_semaphore_create(&timer_sem, (RAW_U8 *)"timer_sem", 0);
		
}

#endif

