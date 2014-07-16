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


/* 	2012-7  Created by jorya_txj
  *	xxxxxx   please added here
  */


#include <raw_api.h>

#if (CONFIG_RAW_QUEUE_SIZE > 0)

/*
************************************************************************************************************************
*                                       Create a queue  
*
* Description: This function is called to create a queue.
*
* Arguments  :p_q is the address of queue size object want to be initialized 
*                 -----
*                  name_ptr  is the queue object name
*                 -----
*                  msg_start is the  start address of msg buffer.
*				      ------ 
*                  number is the number of msgs  of the queue.
* Returns			
*			RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_size_create(RAW_QUEUE_SIZE  *p_q, RAW_U8 *p_name, RAW_MSG_SIZE *msg_start, MSG_SIZE_TYPE number)
{
	RAW_MSG_SIZE      *p_msg1;
    RAW_MSG_SIZE      *p_msg2;

	
	#if (RAW_QUEUE_SIZE_FUNCTION_CHECK > 0)
	

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	if (msg_start == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	if (number == 0) {
		
		return RAW_ZERO_NUMBER;
	}
	
	#endif

	list_init(&p_q->common_block_obj.block_list);
	                             
	p_q->common_block_obj.name = p_name;   
	p_q->common_block_obj.block_way = RAW_BLOCKED_WAY_PRIO;

	p_q->queue_current_msg = 0;
	p_q->peak_numbers = 0;
	
	p_q->queue_msg_size = number;
	p_q->free_msg = msg_start;
	
	/*init the free msg list*/
	p_msg1 = msg_start;
	p_msg2 = msg_start;
	p_msg2++;
   
	while (--number) { 
		
		p_msg1->next = p_msg2;
		p_msg1->msg_ptr = 0;
		p_msg1->msg_size = 0;

		p_msg1++;
		p_msg2++;
	}

	/*init  the last free msg*/ 
	p_msg1->next = 0;                      
	p_msg1->msg_ptr  = 0;
	p_msg1->msg_size = 0;

	p_q->common_block_obj.object_type = RAW_QUEUE_SIZE_OBJ_TYPE;

	TRACE_QUEUE_SIZE_CREATE(raw_task_active, p_q);
	
	return RAW_SUCCESS;
	
}



RAW_U16 msg_size_post(RAW_QUEUE_SIZE *p_q, RAW_MSG_SIZE *p_void,  MSG_SIZE_TYPE size,  RAW_U8 opt_send_method, RAW_U8 opt_wake_all)             
{
	
	LIST *block_list_head;
	
	RAW_MSG_SIZE *msg_temp;
	RAW_MSG_SIZE *p_msg_in; 
	
 	RAW_SR_ALLOC();
	
	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type != RAW_QUEUE_SIZE_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &p_q->common_block_obj.block_list;

	/*queue is full condition!*/
	if (p_q->queue_current_msg >= p_q->queue_msg_size) {  
	
		RAW_CRITICAL_EXIT();

		TRACE_QUEUE_SIZE_MSG_MAX(raw_task_active, p_q, p_void, size, opt_send_method);
		
		return RAW_MSG_MAX;
		
	}

	/*Queue is not full here, If there is no blocked receive task*/
	if (is_list_empty(block_list_head)) {        

		/*delete msg from free msg list*/
		msg_temp             = p_q->free_msg;                  
		p_q->free_msg 	= 	msg_temp->next;
		
		 /* If it is the first message placed in the queue*/
		if (p_q->queue_current_msg == 0) {            
			p_q->write         = msg_temp;                    
			p_q->read        = msg_temp;
			
		} 
		
		else {


			if (opt_send_method == SEND_TO_END)  {

				p_msg_in           = p_q->write;           
				p_msg_in->next  = msg_temp;
				msg_temp->next = 0;
				p_q->write     = msg_temp;

			}

			else {

				msg_temp->next = p_q->read;          
				p_q->read = msg_temp;                 

			}
			
			
		}

		p_q->queue_current_msg++;

		if (p_q->queue_current_msg > p_q->peak_numbers) {

			p_q->peak_numbers = p_q->queue_current_msg;
		}
		
		/*Assign value to msg*/
		msg_temp->msg_ptr = p_void;                               
		msg_temp->msg_size = size;
		
		RAW_CRITICAL_EXIT();

		TRACE_QUEUE_SIZE_MSG_POST(raw_task_active, p_q, p_void, size, opt_send_method);
		
		return RAW_SUCCESS;
	}

	/*wake all the task blocked on this queue*/
	if (opt_wake_all) {

		while (!is_list_empty(block_list_head)) {
			wake_send_msg_size(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list),  p_void, size);

			TRACE_QUEUE_SIZE_WAKE_TASK(raw_task_active, raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list), p_void, size, opt_wake_all);
		}
	}
	
	/*wake hignhest priority task blocked on this queue and send msg to it*/
	else {
		
		wake_send_msg_size(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list),  p_void, size);

		TRACE_QUEUE_SIZE_WAKE_TASK(raw_task_active, raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list), p_void, size, opt_wake_all);
	}
	
	RAW_CRITICAL_EXIT();

	raw_sched();    
	return RAW_SUCCESS;
}





/*
************************************************************************************************************************
*                                    Receive  a msg with size
*
* Description: This function is called to receive a msg with size
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                  msg is the address a point, and this pointer contains address of the msg.
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
*                ------
*                receive_size: is the message size which is to be received
* Returns			
*						RAW_SUCCESS: raw os return success
*						RAW_BLOCK_DEL: if this queue is deleted.
*						RAW_BLOCK_TIMEOUT: queue is still full during waiting time when sending msg.
*						RAW_BLOCK_ABORT:queue is aborted during waiting time when sending msg.
*						RAW_STATE_UNKNOWN: possibly system error.
* Note(s)    	if no msg received then msg will get null pointer(0).ISR can call this function if only wait_option equal RAW_NO_WAIT.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_size_receive(RAW_QUEUE_SIZE *p_q, RAW_TICK_TYPE wait_option, RAW_VOID  **msg_ptr, MSG_SIZE_TYPE *receive_size)
{

	RAW_U16 result;
	
	RAW_MSG_SIZE  *msg_tmp;
	
	RAW_SR_ALLOC();

	
	#if (RAW_QUEUE_SIZE_FUNCTION_CHECK > 0)
	

	if (raw_int_nesting && (wait_option != RAW_NO_WAIT)) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	if (msg_ptr == 0) {
		
		return RAW_NULL_POINTER;
	}

	if (receive_size == 0) {

		return RAW_NULL_POINTER;
	}
	
	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif

	RAW_CRITICAL_ENTER();
	

	if (p_q->common_block_obj.object_type != RAW_QUEUE_SIZE_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	/*if queue has msg then receive it*/
	if (p_q->queue_current_msg) {
      
		msg_tmp =   p_q->read;
		*msg_ptr           = msg_tmp->msg_ptr;
		*receive_size = msg_tmp->msg_size;
	   

	    p_q->read = msg_tmp->next;
		
	    if (p_q->read) {
			
	         p_q->queue_current_msg--;
	    } 

		else {
	       
			p_q->write     = 0;
			p_q->queue_current_msg = 0;
	    }

		
		msg_tmp->next = p_q->free_msg;
		p_q->free_msg =  msg_tmp;
		
		RAW_CRITICAL_EXIT();

		TRACE_QUEUE_SIZE_GET_MSG(raw_task_active, p_q, wait_option, *msg_ptr, *receive_size);
		
		return RAW_SUCCESS;   
	}  
	
	
	if (wait_option == RAW_NO_WAIT) {   
		
		*msg_ptr    = 0;
		*receive_size   = 0;
		
		RAW_CRITICAL_EXIT();

		return RAW_NO_PEND_WAIT;   
	}

	SYSTEM_LOCK_PROCESS_QUEUE_SIZE();

	raw_pend_object((RAW_COMMON_BLOCK_OBJECT  *)p_q, raw_task_active, wait_option);
	
	RAW_CRITICAL_EXIT();

	TRACE_QUEUE_SIZE_GET_BLOCK(raw_task_active, p_q, wait_option);
	
	raw_sched();                                             

	result = block_state_post_process(raw_task_active, 0);

	/*if get the msg successful then take it*/
	if (result == RAW_SUCCESS) {

		*receive_size = raw_task_active->msg_size;
		*msg_ptr =  raw_task_active->msg;
	}

	else {

		*msg_ptr    = 0;
		*receive_size   = 0;

	}
	
	return result;
	
}


/*
************************************************************************************************************************
*                                    Post a msg to the queue front with size
*
* Description: This function is called to post a msg to the queue front and implement LIFO.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                  p_void  is the address of the msg
*               
*                  
*                -------
*               size : is the message size to be posted
*				         
* Returns			
*		RAW_SUCCESS: raw os return success
*		RAW_MSG_MAX:queue is full 
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_size_front_post(RAW_QUEUE_SIZE *p_q, RAW_VOID  *p_void, MSG_SIZE_TYPE size)
{
	#if (RAW_QUEUE_SIZE_FUNCTION_CHECK > 0)
		
		if (p_q == 0) {
			
			return RAW_NULL_OBJECT;
		}
	
		/*if send null pointer, just return*/
		if (p_void == 0) {
			
			return RAW_NULL_POINTER;
		}
		
	#endif
	

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
	
	if (raw_int_nesting && raw_sched_lock) {
		
		return int_msg_post(RAW_TYPE_Q_SIZE_FRONT, p_q, p_void, size, 0, 0);
	}
	
	#endif

	return msg_size_post(p_q, p_void,size,SEND_TO_FRONT, WAKE_ONE_QUEUE);
}



/*
************************************************************************************************************************
*                                    Post a msg to the queue end with size
*
* Description: This function is called to post a msg to the queue end and implement FIFO.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                  p_void  is the address of the msg
*                 -----
*                  wait_option: is  how the service behaves if the msg queue is full.
*							The wait options are
*							defined as follows:
*							RAW_NO_WAIT (0x00000000)
*							RAW_WAIT_FOREVER (0xFFFFFFFF)
*							timeout value (0x00000001
*							through
*							0xFFFFFFFE)
*     		---------
*			size : is the message size to be posted
*				         
* Returns			
*		RAW_SUCCESS: raw os return success
*		RAW_MSG_MAX:queue is full 
*
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_size_end_post(RAW_QUEUE_SIZE *p_q, RAW_VOID  *p_void, MSG_SIZE_TYPE size)
{
	#if (RAW_QUEUE_SIZE_FUNCTION_CHECK > 0)
		
		if (p_q == 0) {
			
			return RAW_NULL_OBJECT;
		}
	
		/*if send null pointer, just return*/
		if (p_void == 0) {
			
			return RAW_NULL_POINTER;
		}
		
	#endif


	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
	
	if (raw_int_nesting && raw_sched_lock) {
		
		return int_msg_post(RAW_TYPE_Q_SIZE_END, p_q, p_void, size, 0, 0);
	}
	
	#endif
	
	return msg_size_post(p_q, p_void, size, SEND_TO_END, WAKE_ONE_QUEUE);
}


/*
************************************************************************************************************************
*                                    Post a msg to the all the task waiting for on this queue with size
*
* Description: This function is called to post a msg to the queue end and implement FIFO or LIFO as opt specified.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                  p_void  is the address of the msg
*                 -----
*                   opt: the opt option  is:
*                    	    SEND_TO_END  implement FIFO
*                    	    SEND_TO_FRONTimplement LIFO
*                  -----	    
*			
*                   size : is the message size to be posted
*
* Returns			
*		RAW_SUCCESS: raw os return success
*		RAW_MSG_MAX:queue is full 
* 
*Note(s)   THis function will wake all the task waiting for this queue other than one!  
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_size_all_post(RAW_QUEUE_SIZE *p_q, RAW_VOID  *p_void, MSG_SIZE_TYPE size, RAW_U8 opt)
{
	#if (RAW_QUEUE_SIZE_FUNCTION_CHECK > 0)
		
		if (p_q == 0) {
			
			return RAW_NULL_OBJECT;
		}
	
		/*if send null pointer, just return*/
		if (p_void == 0) {
			
			return RAW_NULL_POINTER;
		}
		
	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
	
	if (raw_int_nesting) {
		
		return int_msg_post(RAW_TYPE_Q_SIZE_ALL, p_q, p_void, size, 0, opt);
	}
	
	#endif
	
	return msg_size_post(p_q, p_void, size, opt, WAKE_ALL_QUEUE);
}


/*
************************************************************************************************************************
*                                    Check whether queue size obj is full or not
*
* Description: This function is called to Check whether queue size obj is full or not.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*
*
* Returns			
*		1: queue_size obj is full
*		0: queue_size obj is not full
* 
*Note(s)   
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_size_full_check(RAW_QUEUE_SIZE *p_q)
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

	if (p_q->common_block_obj.object_type != RAW_QUEUE_SIZE_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	if (p_q->queue_current_msg >= p_q->queue_msg_size) {   

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
*                                      Flush a queue size
*
* Description: This service deletes all messages stored in the specified message queue.
*						
*              If the queue is empty, this service does nothing.
*
* Arguments  :p_q is the address of the queue object 
*                
* Returns			
*		      RAW_SUCCESS: raw os return success
*
*
* Note(s)     Be careful This api will probably cause memory leak.		
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_QUEUE_SIZE_FLUSH > 0) 
RAW_U16 raw_queue_size_flush(RAW_QUEUE_SIZE  *p_q)
{
	RAW_MSG_SIZE  *p_msg;

	RAW_SR_ALLOC();
	
	
	#if (RAW_QUEUE_SIZE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type != RAW_QUEUE_SIZE_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	if (p_q->queue_current_msg) {
		
        p_msg               	= p_q->write;           
        p_msg->next         	= p_q->free_msg;

		/*free msg reset to queue read*/
        p_q->free_msg       	= p_q->read;         
        
       	p_q->queue_current_msg  = 0;           
        p_q->read          		= 0;
        p_q->write         		= 0;
    }
	
	RAW_CRITICAL_EXIT();

	TRACE_QUEUE_SIZE_FLUSH(raw_task_active, p_q);
	
	return RAW_SUCCESS;
}
#endif



/*
************************************************************************************************************************
*                                      Delete a queue size object
*
* Description: This function is called to delete a queue size object.
*
* Arguments  :p_q is the address of this queue object
*                 -----
*                  All blocked task will be waked up and receive a RAW_BLOCK_DEL
*				         
* Returns:			
*		RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_QUEUE_SIZE_DELETE > 0)
RAW_U16 raw_queue_size_delete(RAW_QUEUE_SIZE *p_q)
{
	LIST  *block_list_head;
	
	RAW_SR_ALLOC();

	#if (RAW_QUEUE_SIZE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type != RAW_QUEUE_SIZE_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &p_q->common_block_obj.block_list;
	
	p_q->common_block_obj.object_type = 0;
	/*All task blocked on this queue is waken up*/
	while (!is_list_empty(block_list_head))  {
		delete_pend_obj(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list));	
	}                             
	
	RAW_CRITICAL_EXIT();

	TRACE_QUEUE_SIZE_DELETE(raw_task_active, p_q);

	raw_sched(); 
	
	return RAW_SUCCESS;
	
}
#endif


/*
************************************************************************************************************************
*                                     Get queue size information
*
* Description: This function is called to get information form queue.
*
* Arguments  :p_q is the address of this queue object
*                 -----
*                queue_free_msg_size will be filled with free numbers of queue size msg       
*		     -----
*               queue_peak_msg_size will be filled with the max used numbers of queue size msg.
*               -------
*               queue_current_msg will be filled with the current used numbers of queue size msg.  			         
* Returns			
*              RAW_SUCCESS: raw os return success
*              RAW_NOT_CALLED_BY_ISR: not called by isr when CONFIG_RAW_ZERO_INTERRUPT is open.
* Note(s)    	
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_QUEUE_SIZE_GET_INFORMATION > 0)
RAW_U16 raw_queue_size_get_information(RAW_QUEUE_SIZE *p_q, MSG_SIZE_TYPE *queue_free_msg_size, MSG_SIZE_TYPE *queue_peak_msg_size, MSG_SIZE_TYPE *queue_current_msg)
{

	RAW_SR_ALLOC();
	
	#if (RAW_QUEUE_SIZE_FUNCTION_CHECK > 0)

	if (p_q == 0)  {
		
		return RAW_NULL_OBJECT;
	}
	
	if (queue_free_msg_size == 0) {
		
		return RAW_NULL_POINTER;
	}

	if (queue_current_msg == 0) {
		
		return RAW_NULL_POINTER;
	}

	
	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif

	RAW_CRITICAL_ENTER();

	if (p_q->common_block_obj.object_type != RAW_QUEUE_SIZE_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}
	
	*queue_free_msg_size = p_q->queue_msg_size - p_q->queue_current_msg;
	*queue_current_msg = p_q->queue_current_msg;
	*queue_peak_msg_size = p_q->peak_numbers;
	
	RAW_CRITICAL_EXIT();

	return RAW_SUCCESS;

}

#endif

#endif

