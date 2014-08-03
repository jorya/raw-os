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


/* 	2012-10  Created by jorya_txj
  *	xxxxxx   please added here
  */
  

#include <raw_api.h>


#if (CONFIG_RAW_TASK_0 > 0)

static void task_0_tick_handler(TASK_0_EVENT_TYPE ev, void *event_data)
{
	event_data = event_data;

	#if (CONFIG_RAW_USER_HOOK > 0)
	raw_tick_hook();
	#endif

	/*update system time to calculate whether task timeout happens*/
	#if (CONFIG_RAW_TICK_TASK > 0)
	raw_task_semaphore_put(&tick_task_obj);
	#else
	tick_list_update();
	#endif

	/*update task time slice if possible*/
	#if (CONFIG_SCHED_FIFO_RR > 0)
	calculate_time_slice(ev);
	#endif
	
	/*inform the timer task to update software timer*/	
	#if (CONFIG_RAW_TIMER > 0)
	call_timer_task();
	#endif

}

/*
************************************************************************************************************************
*                                       Timer tick function 
*
* Description: This function is called by timer interrupt if CONFIG_RAW_TASK_0 is enabled.
*
* Arguments  :None
*                
*                 
*
*				         
* Returns		 None
*						
* Note(s)    Called by your own system timer interrupt.
*
*             
************************************************************************************************************************
*/
RAW_U16 task_0_tick_post(void)
{
	RAW_U16 ret;
	
	ret = raw_task_0_post(&task_0_event_handler, raw_task_active->priority, 0);
	
	return ret;
}


static RAW_U16 task_0_post(EVENT_HANLDER *p, TASK_0_EVENT_TYPE ev, void *event_data, RAW_U8 opt_send_method)
{
	RAW_U16 task_0_event_position;
	RAW_SR_ALLOC();

	/*this function should not be called in task*/
	if (raw_int_nesting == 0) {

		return RAW_NOT_CALLED_BY_TASK;
	}

	/*fastest way to make task 0 ready*/
	RAW_CPU_DISABLE();

	/*if message is max, probally interrupt is too fast, please check your interrupt*/
	if(task_0_events == MAX_TASK_EVENT) {
		RAW_CPU_ENABLE();
		TRACE_TASK_0_OVERFLOW(p, ev, event_data);
		return RAW_TASK_0_EVENT_EXHAUSTED;
	}

	++task_0_events;

	/*Update the debug information*/
	if (task_0_events > peak_events) {
		peak_events = task_0_events;
	}
	
	if (opt_send_method == SEND_TO_END) {

		task_0_event_position = task_0_event_head;
		
		task_0_event_head++;

		if (task_0_event_head == MAX_TASK_EVENT) {   
			
			task_0_event_head = 0;
			
		}
	}

	else {

		if (task_0_event_end == 0) { 			   
			task_0_event_end = MAX_TASK_EVENT;
		}

		task_0_event_end--;

		task_0_event_position = task_0_event_end;

	}

	/*Store the message*/
	task_0_events_queue[task_0_event_position].ev = ev;
	task_0_events_queue[task_0_event_position].event_data = event_data;
	task_0_events_queue[task_0_event_position].p = p;

	RAW_CPU_ENABLE();

	return RAW_SUCCESS;
 
}

/*
************************************************************************************************************************
*                                       Post an event to end of task 0 queue
*
* Description: This function is called to post an event to end of task 0 queue and implement FIFO.
*
* Arguments  : p is which event handler         
*                   ---------------------
*			ev is the event signal 
*                   ---------------------
*                   data is the event data
* Returns		
*						
* Note(s)   this function should be called in interrupt to trigger task 0. 
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_0_post(EVENT_HANLDER *p, TASK_0_EVENT_TYPE ev, void *event_data)
{

	return task_0_post(p, ev, event_data, SEND_TO_END);
}


/*
************************************************************************************************************************
*                                       Post an event to front of task 0 queue
*
* Description: This function is called to post an event to front of task 0 queue and implement LIFO.
*
* Arguments  : p is which event handler         
*                   ---------------------
*			ev is the event signal 
*                   ---------------------
*                   data is the event data
* Returns		
*						
* Note(s)   this function should be called in interrupt to trigger task 0. 
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_0_front_post(EVENT_HANLDER *p, TASK_0_EVENT_TYPE ev, void *event_data)
{

	return task_0_post(p, ev, event_data, SEND_TO_FRONT);
}


static void task_0_process(void *pa)
{
	TASK_0_EVENT_TYPE ev;
	RAW_U8 *data_temp;
	EVENT_HANLDER *receiver;
	
	RAW_SR_ALLOC();

	pa = pa;

	/*to prevent interrupt happen here to cause system crash at task 0 start*/
	USER_CPU_INT_DISABLE();
	
	remove_ready_list(&raw_ready_queue, &raw_task_0_obj);
	
	USER_CPU_INT_ENABLE();
	
	while (1) {

		/*Get the message info and update it*/
		USER_CPU_INT_DISABLE();

		if (task_0_events) {

			--task_0_events;
			
			/* There are events that we should deliver. */
			ev = task_0_events_queue[task_0_event_end].ev;

			data_temp = task_0_events_queue[task_0_event_end].event_data;
			receiver = task_0_events_queue[task_0_event_end].p;

			task_0_event_end++;

			if (task_0_event_end == MAX_TASK_EVENT) {                  
			    task_0_event_end = 0u;
			}

			/*lock the scheduler, so event handler can not be switched out*/
			raw_sched_lock = 1u;
			
			USER_CPU_INT_ENABLE();

			/*exceute the event handler*/
			receiver->handle_event(ev, data_temp);
		}

		else {

			/*unlock the scheduler, so scheduler can work*/
			raw_sched_lock = 0u;
			
			get_ready_task(&raw_ready_queue);
			CONTEXT_SWITCH();

			USER_CPU_INT_ENABLE();

		}
			
	}
		
}

void hybrid_int_process(void)
{
	TASK_0_EVENT_TYPE hybrid_ev;
	RAW_U8 *hybrid_data_temp;
	EVENT_HANLDER *hybrid_receiver;
	LIST *hybrid_node;
	RAW_U8 hybrid_highest_pri;

	RAW_SR_ALLOC();
	
	register RAW_U8 hybrid_task_may_switch = 0u;

	while (1) {
		
		USER_CPU_INT_DISABLE(); 
		
		if (task_0_events) {

			/*current running task can never be task 0*/
			if (raw_int_nesting) {

				raw_sched_lock = 0;
				USER_CPU_INT_ENABLE();
				return;
			}

			else {

				--task_0_events;
				/* There are events that we should deliver. */
				hybrid_ev = task_0_events_queue[task_0_event_end].ev;
				hybrid_data_temp = task_0_events_queue[task_0_event_end].event_data;
				hybrid_receiver = task_0_events_queue[task_0_event_end].p;

				task_0_event_end++;

				if (task_0_event_end == MAX_TASK_EVENT) {                  
					task_0_event_end = 0;
				}
				
				USER_CPU_INT_ENABLE();

				/*exceute the event handler*/
				hybrid_receiver->handle_event(hybrid_ev, hybrid_data_temp);
				hybrid_task_may_switch = 1;
			}
			
		}

		else {
			
			raw_sched_lock = 0;

			if (hybrid_task_may_switch) {
				
				hybrid_highest_pri = raw_ready_queue.highest_priority;
				/*Highest priority task must be the first element on the list*/
				hybrid_node = raw_ready_queue.task_ready_list[hybrid_highest_pri].next;

				/*Get the highest priority task object*/
				high_ready_obj = raw_list_entry(hybrid_node, RAW_TASK_OBJ, task_list);

				/*if highest task is currently task, then no need to do switch and just return*/
				if (high_ready_obj == raw_task_active) { 
					
					USER_CPU_INT_ENABLE();                                     
					return;

				}

				CONTEXT_SWITCH();
			}
			
			USER_CPU_INT_ENABLE();
			return;

		}

	}
	
		
}



#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

static void int_msg_handler(TASK_0_EVENT_TYPE ev, void *msg_data)
{
	OBJECT_INT_MSG *int_msg;
	RAW_U16 int_msg_ret;
	
	RAW_SR_ALLOC();
	
	int_msg = msg_data;
	int_msg_ret = RAW_SYSTEM_ERROR;
	
	switch (ev) {

		#if (CONFIG_RAW_TASK_SUSPEND > 0)
		
		case RAW_TYPE_SUSPEND:
			int_msg_ret = task_suspend((RAW_TASK_OBJ *)(int_msg->object));
			break;

			
		#endif
		
		#if (CONFIG_RAW_TASK_RESUME > 0)
		
		case RAW_TYPE_RESUME:
			int_msg_ret = task_resume((RAW_TASK_OBJ *)(int_msg->object));
			break;

			
		#endif

		#if (CONFIG_RAW_SEMAPHORE > 0)
		
		case RAW_TYPE_SEM:
			int_msg_ret = semaphore_put((RAW_SEMAPHORE *)(int_msg->object), WAKE_ONE_SEM);
			break;

		case RAW_TYPE_SEM_ALL:
			int_msg_ret = semaphore_put((RAW_SEMAPHORE *)(int_msg->object), WAKE_ALL_SEM);
			break;
			
		#endif
		
		
		#if (CONFIG_RAW_QUEUE > 0)
		
		case RAW_TYPE_Q_FRONT:
			int_msg_ret = msg_post((RAW_QUEUE *)(int_msg->object), int_msg->msg, SEND_TO_FRONT, WAKE_ONE_QUEUE);
			break;

		case RAW_TYPE_Q_END:
			int_msg_ret = msg_post((RAW_QUEUE *)(int_msg->object), int_msg->msg, SEND_TO_END, WAKE_ONE_QUEUE);
			break;

		case RAW_TYPE_Q_ALL:
			int_msg_ret = msg_post((RAW_QUEUE *)(int_msg->object), int_msg->msg, int_msg->opt, WAKE_ALL_QUEUE);
			break;

		#endif


		#if (CONFIG_RAW_QUEUE_SIZE > 0)
		
		case RAW_TYPE_Q_SIZE_FRONT:
			int_msg_ret = msg_size_post((RAW_QUEUE_SIZE *)(int_msg->object), int_msg->msg, int_msg->msg_size, SEND_TO_FRONT, WAKE_ONE_QUEUE);
			break;

		case RAW_TYPE_Q_SIZE_END:
			int_msg_ret = msg_size_post((RAW_QUEUE_SIZE *)(int_msg->object), int_msg->msg, int_msg->msg_size, SEND_TO_END, WAKE_ONE_QUEUE);
			break;

		case RAW_TYPE_Q_SIZE_ALL:
			int_msg_ret = msg_size_post((RAW_QUEUE_SIZE *)(int_msg->object), int_msg->msg, int_msg->msg_size, int_msg->opt, WAKE_ALL_QUEUE);
			break;

		#endif

		#if (CONFIG_RAW_EVENT > 0)
		
		case RAW_TYPE_EVENT:
			int_msg_ret = event_set((RAW_EVENT *)(int_msg->object), int_msg->event_flags, int_msg->opt);
			break;
			
		#endif

		#if (CONFIG_RAW_IDLE_EVENT > 0)
		
		case RAW_TYPE_IDLE_END_EVENT_POST:
			int_msg_ret = event_post((ACTIVE_EVENT_STRUCT *)(int_msg->object), int_msg->msg_size, int_msg->msg, SEND_TO_END);
			break;

		case RAW_TYPE_IDLE_FRONT_EVENT_POST:
			int_msg_ret = event_post((ACTIVE_EVENT_STRUCT *)(int_msg->object), int_msg->msg_size, int_msg->msg, SEND_TO_FRONT);
			break;
			
		#endif
		
		default:
			RAW_ASSERT(0);



	}

	if (int_msg_ret != RAW_SUCCESS) {

		TRACE_INT_MSG_HANDLE_ERROR(ev, int_msg_ret);
		RAW_ASSERT(0);
	}

	RAW_CPU_DISABLE();

	int_msg->next = free_object_int_msg;
	free_object_int_msg = int_msg;
	
	RAW_CPU_ENABLE();

}



static void int_msg_init(void)
{
	OBJECT_INT_MSG *p_msg1;
	OBJECT_INT_MSG *p_msg2;
	RAW_U32 number;

	msg_event_handler.handle_event = int_msg_handler;

	number = OBJECT_INT_MSG_SIZE;

	raw_memset(object_int_msg, 0, sizeof(object_int_msg));
	
	free_object_int_msg = object_int_msg;
	
	/*init the free msg list*/
	p_msg1 = object_int_msg;
	p_msg2 = object_int_msg;
	p_msg2++;

	while (--number) { 

		p_msg1->next = p_msg2;

		p_msg1++;
		p_msg2++;
	}

	/*init  the last free msg*/ 
	p_msg1->next = 0;                      
	
}



RAW_U16 int_msg_post(RAW_U8 type, void *p_obj, void *p_void, MSG_SIZE_TYPE msg_size, RAW_U32 flags, RAW_U8 opt)
{
	void *msg_data;

	RAW_SR_ALLOC();

	RAW_CPU_DISABLE();
	
	if (free_object_int_msg == 0) {
		
		int_msg_full++;
		
		RAW_CPU_ENABLE();
		
		TRACE_INT_MSG_EXHAUSTED();
		
		return RAW_INT_MSG_EXHAUSTED;
	}

	msg_data = free_object_int_msg;
	
	free_object_int_msg->type = type;
	free_object_int_msg->object = p_obj;
	free_object_int_msg->msg = p_void;
	free_object_int_msg->msg_size = msg_size;
	free_object_int_msg->event_flags = flags;
	free_object_int_msg->opt = opt;
	
	free_object_int_msg = free_object_int_msg->next;

	RAW_CPU_ENABLE();

	TRACE_INT_MSG_POST(type, p_obj, p_void, msg_size, flags, opt);
	
	return raw_task_0_post(&msg_event_handler, type, msg_data);

}


#endif                  


/*
************************************************************************************************************************
*                                      Init task 0
*
* Description: This function is called to init task 0
*
* Arguments  :NONE   
*
* Returns		
*						
* Note(s)     
*
*             
************************************************************************************************************************
*/	
void raw_task_0_init(void)
{

	/*Create task 0  to handle fast interrupt event*/
	raw_task_create(&raw_task_0_obj, (RAW_U8  *)"task_0_object",  0, 
	0,  0, task_0_stack, TASK_0_STACK_SIZE, task_0_process, 1);

	task_0_event_handler.handle_event = task_0_tick_handler;
	
	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
	int_msg_init();
	#endif
}

#endif

