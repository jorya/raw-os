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

#if (CONFIG_RAW_QUEUE > 0)

/*
************************************************************************************************************************
*                                       Create a queue 
*
* Description: This function is called to create a queue.
*
* Arguments  :p_q is the address of queue object want to be initialized 
*                 -----
*                  name_ptr  is the queue object name
*                 -----
*                  msg_start is the address of pointer array.
*				      ------ 
*                  number is the number of elements  of the queue.
* Returns			
*		      RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_create(RAW_QUEUE *p_q, RAW_U8 *p_name, RAW_VOID **msg_start, MSG_SIZE_TYPE number)
{
	
	
	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	if (msg_start == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	if (number == 0u) {
		
		return RAW_ZERO_NUMBER;
	}
	
	#endif

	/*init the queue blocked list*/
	list_init(&p_q->common_block_obj.block_list);
	                             
	p_q->common_block_obj.name = p_name;   
	p_q->common_block_obj.block_way = RAW_BLOCKED_WAY_PRIO;
	p_q->msg_q.queue_start 		= msg_start;               /*      Initialize the queue                 */
	p_q->msg_q.queue_end        = &msg_start[number];
	p_q->msg_q.write            = msg_start;
	p_q->msg_q.read             = msg_start;
	p_q->msg_q.size            	= number;
	p_q->msg_q.current_numbers  = 0u;
	p_q->msg_q.peak_numbers		= 0u;
	p_q->queue_send_notify      = 0;
	p_q->common_block_obj.object_type = RAW_QUEUE_OBJ_TYPE;

	TRACE_QUEUE_CREATE(raw_task_active, p_q); 
	
	return RAW_SUCCESS;
}


RAW_U16 msg_post(RAW_QUEUE *p_q, RAW_VOID *p_void, RAW_U8 opt_send_method, RAW_U8 opt_wake_all)             
{
	LIST *block_list_head;

 	RAW_SR_ALLOC();

	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type != RAW_QUEUE_OBJ_TYPE) {
		
		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &p_q->common_block_obj.block_list;
	
	if (p_q->msg_q.current_numbers >= p_q->msg_q.size) {  

		RAW_CRITICAL_EXIT();
		
		TRACE_QUEUE_MSG_MAX(raw_task_active, p_q, p_void, opt_send_method); 
		
		return RAW_MSG_MAX;
	}


	/*Queue is not full here, if there is no blocked receive task*/
	if (is_list_empty(block_list_head)) {        

		p_q->msg_q.current_numbers++;                                

		/*update peak_numbers for debug*/
		if (p_q->msg_q.current_numbers > p_q->msg_q.peak_numbers) {

			p_q->msg_q.peak_numbers = p_q->msg_q.current_numbers;
		}
		
		if (opt_send_method == SEND_TO_END)  {

			*p_q->msg_q.write++ = p_void;                              

			if (p_q->msg_q.write == p_q->msg_q.queue_end) {   
				
				p_q->msg_q.write = p_q->msg_q.queue_start;
				
			}   

		}

		else {

			 /* Wrap read pointer to end if we are at the 1st queue entry */
			if (p_q->msg_q.read == p_q->msg_q.queue_start) {                
	        	p_q->msg_q.read = p_q->msg_q.queue_end;
	    	}
			
			p_q->msg_q.read--;
			*p_q->msg_q.read = p_void;                               /* Insert message into queue                     */
			
		}
		
		RAW_CRITICAL_EXIT();

		/*if queue is registered with notify function just call it*/		
		if (p_q->queue_send_notify) {

			p_q->queue_send_notify(p_q);	
		}

		TRACE_QUEUE_MSG_POST(raw_task_active, p_q, p_void, opt_send_method); 
		
		return RAW_SUCCESS;
	}

	/*wake all the task blocked on this queue*/
	if (opt_wake_all) {

		while (!is_list_empty(block_list_head)) {
			
			wake_send_msg(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list),  p_void);	
			
			TRACE_QUEUE_WAKE_TASK(raw_task_active, raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list), p_void, opt_wake_all);
			
		}
		
	}
	
	/*wake hignhest priority task blocked on this queue and send msg to it*/
	else {
		
		wake_send_msg(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list),  p_void);
		
		TRACE_QUEUE_WAKE_TASK(raw_task_active, raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list), p_void, opt_wake_all);

	}
	
	RAW_CRITICAL_EXIT();

	raw_sched();    
	return RAW_SUCCESS;
}




/*
************************************************************************************************************************
*                                    Post a msg to the queue front
*
* Description: This function is called to post a msg to the queue front and implement LIFO.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                  p_void  is the address of the msg
*                 -----
*                  
*				         
* Returns			
*		RAW_SUCCESS: raw os return success
*		RAW_MSG_MAX:queue is full 
*		
*		
*		
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_front_post(RAW_QUEUE *p_q, RAW_VOID *p_void)
{

	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	
	if (p_void == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	#endif

	TRACE_QUEUE_FP_TIME_RECORD(p_q, p_void);
	
	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting && raw_sched_lock) {
		
		return int_msg_post(RAW_TYPE_Q_FRONT, p_q, p_void, 0, 0, 0);
	}
	
	#endif
	
	return msg_post(p_q, p_void,SEND_TO_FRONT, WAKE_ONE_QUEUE);
}


						
/*
************************************************************************************************************************
*                                    Post a msg to the queue end
*
* Description: This function is called to post a msg to the queue end and implement FIFO.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                  p_void  is the address of the msg
*                 -----
*                 
*				         
* Returns			
*			RAW_SUCCESS: raw os return success
*			RAW_MSG_MAX:queue is full 
*			
*			
*			
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_end_post(RAW_QUEUE *p_q, RAW_VOID *p_void)
{
	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	
	if (p_void == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	#endif

	TRACE_QUEUE_EP_TIME_RECORD(p_q, p_void);
	
	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
	
	if (raw_int_nesting && raw_sched_lock) {
		
		return int_msg_post(RAW_TYPE_Q_END, p_q, p_void, 0, 0, 0);
	}
	
	#endif
	
	return msg_post(p_q, p_void,  SEND_TO_END, WAKE_ONE_QUEUE);
	
}


/*
************************************************************************************************************************
*                                   Notify function call back 
*
* Description: This function is called to post a msg to the queue end and implement FIFO.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                  p_void  is the address of the msg
*                 -----
*                 
*				         
* Returns			
*			RAW_SUCCESS: raw os return success
*			RAW_MSG_MAX:queue is full 
*			
*			
*			
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_post_notify(RAW_QUEUE *p_q, RAW_VOID *p_void)
{
	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	
	if (p_void == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
	
	if (raw_int_nesting) {
		
		return int_msg_post(RAW_TYPE_Q_END, p_q, p_void, 0, 0, 0);
	}
	
	#endif
	
	return msg_post(p_q, p_void,  SEND_TO_END, WAKE_ONE_QUEUE);
	
}


/*
************************************************************************************************************************
*                                    Post a msg to the all the task waiting for on this queue
*
* Description: This function is called to post a msg to the queue end and implement FIFO or LIFO as opt specified.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                  p_void  is the address of the msg
*                 -----
*                  opt: the opt option  is:
*                    	   SEND_TO_END  implement FIFO
*                    	   SEND_TO_FRONTimplement LIFO
*                  -----	    
*                 
*				         
* Returns	RAW_SUCCESS: raw os return success	
*			RAW_MSG_MAX:queue is full 
*
* Note(s)   THis function will wake all the task waiting for this queue other than one!  
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_all_post(RAW_QUEUE *p_q, RAW_VOID *p_void, RAW_U8 opt)
{
	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	
	if (p_void == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	#endif

	TRACE_QUEUE_AP_TIME_RECORD(p_q, p_void, opt);
	
	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
	
	if (raw_int_nesting) {
		
		return int_msg_post(RAW_TYPE_Q_ALL, p_q, p_void, 0, 0, opt);
	}
	
	#endif
	
	
	return msg_post(p_q, p_void, opt, WAKE_ALL_QUEUE);
	
}


/*
************************************************************************************************************************
*                                    Register notify function to queue
*
* Description: This function is called to Register notify function to queue.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                   notify_function is the function to be called whennever send queue data to it.
*                 -----
*                 
*				         
* Returns			
*			RAW_SUCCESS: raw os return success
*					
* Note(s)       This function is normally used to implement pending on multi object function.	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_send_notify(RAW_QUEUE *p_q, QUEUE_SEND_NOTIFY notify_function)
{

	RAW_SR_ALLOC();
	

	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type != RAW_QUEUE_OBJ_TYPE) {
		
		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}
	
	p_q->queue_send_notify = notify_function;
	RAW_CRITICAL_EXIT();
	
	return RAW_SUCCESS;
}


/*
************************************************************************************************************************
*                                    Receive  a msg
*
* Description: This function is called to receive a msg
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                  msg is the address of a point, and this pointer contains address of the msg.
*                  -----	    
*                  wait_option: is  how the service behaves if the msg queue is full.
*							The wait options are
*							defined as follows:
*							RAW_NO_WAIT (0x00000000)
*							RAW_WAIT_FOREVER (0xFFFFFFFF)
*							timeout value (0x00000001
*							through
*							0xFFFFFFFE)
*				         
* Returns			
*						RAW_SUCCESS: raw os return success
*						RAW_BLOCK_DEL: if this queue is deleted.
*						RAW_BLOCK_TIMEOUT: queue is still full during waiting time when sending msg.
*						RAW_BLOCK_ABORT:queue is aborted during waiting time when sending msg.
*						RAW_STATE_UNKNOWN: possibly system error.
* Note(s)    	if no msg received then msg will get null pointer(0). ISR can call this function if only wait_option equal RAW_NO_WAIT.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_receive(RAW_QUEUE *p_q, RAW_TICK_TYPE wait_option, RAW_VOID **msg)
{

	RAW_VOID *pmsg;
	RAW_U16 result;
	
	RAW_SR_ALLOC();

	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (raw_int_nesting && (wait_option != RAW_NO_WAIT)) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	
	if (msg == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type != RAW_QUEUE_OBJ_TYPE) {
		
		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

  	/*if queue has msgs, just receive it*/
	if (p_q->msg_q.current_numbers) { 
		
		pmsg = *p_q->msg_q.read++;                    
		
		if (p_q->msg_q.read == p_q->msg_q.queue_end) { 
			/*wrap around to start*/
			p_q->msg_q.read = p_q->msg_q.queue_start;
		}

		*msg = pmsg;

		p_q->msg_q.current_numbers--;  
		
		RAW_CRITICAL_EXIT();

		TRACE_QUEUE_GET_MSG(raw_task_active, p_q, wait_option, *msg);
		
		return RAW_SUCCESS;                         
	}



	if (wait_option == RAW_NO_WAIT) {   
		*msg = (RAW_VOID *)0;
		RAW_CRITICAL_EXIT();
		return RAW_NO_PEND_WAIT;
	} 

	/*if system is locked, block operation is not allowed*/
	SYSTEM_LOCK_PROCESS_QUEUE();

	raw_pend_object((RAW_COMMON_BLOCK_OBJECT  *)p_q, raw_task_active, wait_option);
	
	RAW_CRITICAL_EXIT();

	TRACE_QUEUE_GET_BLOCK(raw_task_active, p_q, wait_option);
	
	raw_sched();                                             

	*msg      = (RAW_VOID      *)0;
	result = block_state_post_process(raw_task_active, msg);
	
	return result;
	
}

/*
************************************************************************************************************************
*                                    Check whether queue  obj is full or not
*
* Description: This function is called to Check whether queue obj is full or not.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*
*
* Returns			
*		1: queue obj is full
*		0: queue obj is not full
* 
*Note(s)   
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_full_check(RAW_QUEUE *p_q)
{
	RAW_SR_ALLOC();

	RAW_U16 full_check_ret;
	
	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {

		return RAW_NULL_OBJECT;
	}

	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif

	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type != RAW_QUEUE_OBJ_TYPE) {
		
		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	if (p_q->msg_q.current_numbers >= p_q->msg_q.size) {  

		full_check_ret = 1u;
	}

	else {

		full_check_ret = 0u;

	}

	RAW_CRITICAL_EXIT();

	return full_check_ret;

}


/*
************************************************************************************************************************
*                                      Flush a queue 
*
* Description: This service deletes all messages stored in the specified message queue.
*              If the queue is empty, this service does nothing.
*
* Arguments  :p_q is the address of the queue object 
*                    -----
* 
*
*Returns      RAW_SUCCESS: raw os return success
*
*
* Note(s)     Be careful This api will probably cause memory leak.	
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_QUEUE_FLUSH > 0) 
RAW_U16 raw_queue_flush(RAW_QUEUE  *p_q)
{
	RAW_SR_ALLOC();
	

	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type != RAW_QUEUE_OBJ_TYPE) {
		
		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	/*Reset  read and write pointer to init position*/
	p_q->msg_q.write = p_q->msg_q.queue_start;
	p_q->msg_q.read  = p_q->msg_q.queue_start;

	p_q->msg_q.current_numbers = 0u;
	
	RAW_CRITICAL_EXIT(); 

	TRACE_QUEUE_FLUSH(raw_task_active, p_q);
	return RAW_SUCCESS;
}
#endif


/*
************************************************************************************************************************
*                                      Delete a queue 
*
* Description: This function is called to delete a queue.
*
* Arguments  :p_q is the address of this queue object
*                 -----
*                  All blocked task will be waken up and return RAW_BLOCK_DEL.
*				         
* Returns			
*			RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_QUEUE_DELETE > 0)
RAW_U16 raw_queue_delete(RAW_QUEUE *p_q)
{
	LIST  *block_list_head;
	
	RAW_SR_ALLOC();

	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type != RAW_QUEUE_OBJ_TYPE) {
		
		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &p_q->common_block_obj.block_list;
	
	p_q->common_block_obj.object_type = 0u;
	
	/*All task blocked on this queue is waken up*/
	while (!is_list_empty(block_list_head))  {
		delete_pend_obj(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list));	
	}                             
	
	RAW_CRITICAL_EXIT();

	TRACE_QUEUE_DELETE(raw_task_active, p_q);

	raw_sched(); 
	
	return RAW_SUCCESS;
	
}
#endif


/*
************************************************************************************************************************
*                                     Get queue information
*
* Description: This function is called to get information form queue.
*
* Arguments  :p_q is the address of this queue object
*                 -----
*                  msg_information  is the address of RAW_MSG_INFO, which will be filled queue information 
*                 -----
*                 
*				         
* Returns			
*			RAW_SUCCESS: raw os return success
*                   RAW_NOT_CALLED_BY_ISR: not called by isr when CONFIG_RAW_ZERO_INTERRUPT is open.
* Note(s)    	Commonly for debug purpose
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_QUEUE_GET_INFORMATION > 0)
RAW_U16 raw_queue_get_information(RAW_QUEUE *p_q, RAW_MSG_INFO *msg_information)
{
	LIST *block_list_head;
	
	RAW_SR_ALLOC();
	
	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0)  {
		
		return RAW_NULL_OBJECT;
	}

	
	if (msg_information == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type !=  RAW_QUEUE_OBJ_TYPE) {
		
		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &p_q->common_block_obj.block_list;
	
	msg_information->msg_q.peak_numbers =  p_q->msg_q.peak_numbers;
	msg_information->msg_q.current_numbers = p_q->msg_q.current_numbers;
	msg_information->msg_q.queue_start =  p_q->msg_q.queue_start;
	msg_information->msg_q.queue_end = p_q->msg_q.queue_end;
	msg_information->msg_q.read = p_q->msg_q.read;
	msg_information->msg_q.write = p_q->msg_q.write;
	msg_information->msg_q.size = p_q->msg_q.size;
	msg_information->suspend_entry = block_list_head->next;
	
	RAW_CRITICAL_EXIT();

	return RAW_SUCCESS;

}
#endif


#endif

