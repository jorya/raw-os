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


/* 	2013-6  Created by jorya_txj
  *	xxxxxx   please added here
  */


#include <raw_api.h>

#if (CONFIG_RAW_QUEUE_BUFFER > 0)

/*
 * Check message buffer free space
 *	If 'msgsz' message is able to be stored, return TRUE.
 */
RAW_INLINE RAW_BOOLEAN is_queue_buffer_free(RAW_QUEUE_BUFFER *q_b, MSG_SIZE_TYPE msg_size)
{
	return ((HEADERSZ + msg_size) <= q_b->frbufsz);
}

/*
 * If message buffer is empty, return TRUE.
 */
RAW_INLINE RAW_BOOLEAN is_buffer_empty(RAW_QUEUE_BUFFER *q_b)
{
	return (q_b->frbufsz == q_b->bufsz);
}


/*
************************************************************************************************************************
*                                       Create a queue buffer
*
* Description: This function is called to create a queue buffer.
*
* Arguments  :p_q is the address of queue buffer object want to be initialized 
*                 -----
*                  p_name  is the queue buffer object name
*                 -----
*                  msg_buffer is the start address of queue buffer.
*				      ------ 
*                 buffer_size is the size of of the queue buffer.
* Returns			
*		      RAW_SUCCESS: raw os return success
*			RAW_QUEUE_BUFFER_SIZE_0: queue buffer size 0.
*			RAW_QUEUE_BUFFER_INVALID_SIZE:invalid buffer size.
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_buffer_create(RAW_QUEUE_BUFFER *q_b, RAW_U8 *p_name, RAW_VOID *msg_buffer, MSG_SIZE_TYPE buffer_size, MSG_SIZE_TYPE max_msg_size)
{
	MSG_SIZE_TYPE bufsz;
	RAW_U8        queue_buffer_align_mask;

	#if (RAW_QUEUE_BUFFER_FUNCTION_CHECK > 0)

	if (q_b == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	if (msg_buffer == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	#endif
	
	bufsz = ROUND_BUFFER_SIZE(buffer_size);

	if (bufsz == 0) {

		return RAW_QUEUE_BUFFER_SIZE_0;
		
	}

	
	if (bufsz != buffer_size) {
		
		return RAW_QUEUE_BUFFER_INVALID_SIZE;
	}

	queue_buffer_align_mask = HEADERSZ - 1u;

	if (((RAW_U32)msg_buffer & queue_buffer_align_mask)){                             

		return RAW_INVALID_ALIGN;

	}

	/*init the queue blocked list*/
	list_init(&q_b->common_block_obj.block_list);

	q_b->bufsz = bufsz;
	q_b->frbufsz = bufsz;
	q_b->buffer = msg_buffer;
	q_b->maxmsz = max_msg_size;
	q_b->head = 0;
	q_b->tail = 0;
	
	q_b->common_block_obj.name = p_name;
	q_b->common_block_obj.block_way = RAW_BLOCKED_WAY_PRIO;
	q_b->common_block_obj.object_type = RAW_QUEUE_BUFFER_OBJ_TYPE;

	TRACE_QUEUE_BUFFER_CREATE(raw_task_active, q_b); 
	
	return RAW_SUCCESS;

}


static void msg_to_end_buffer(RAW_QUEUE_BUFFER *q_b, void *msg, MSG_SIZE_TYPE msgsz)
{
	MSG_SIZE_TYPE	tail = q_b->tail;
	RAW_U8	*buffer = q_b->buffer;
	MSG_SIZE_TYPE	remsz;

	q_b->frbufsz -= (HEADERSZ + ROUND_BUFFER_SIZE(msgsz));

	*(HEADER*)&buffer[tail] = msgsz;
	tail += HEADERSZ;
	if (tail >= q_b->bufsz) {
		tail = 0;
	}

	if ((remsz = q_b->bufsz - tail) < msgsz) {
		raw_memcpy(&buffer[tail], msg, remsz);
		msg = (RAW_U8 *)msg + remsz;
		msgsz -= remsz;
		tail = 0;
	}
	
	raw_memcpy(&buffer[tail], msg, msgsz);
	tail += ROUND_BUFFER_SIZE(msgsz);
	
	if (tail >= q_b->bufsz) {
		tail = 0;
	}

	q_b->tail = tail;
	
}


static RAW_U32 buffer_to_msg(RAW_QUEUE_BUFFER *q_b, void *msg)
{
	MSG_SIZE_TYPE	head = q_b->head;
	RAW_U8	*buffer = q_b->buffer;
	MSG_SIZE_TYPE	msgsz, actsz;
	MSG_SIZE_TYPE	remsz;

	actsz = msgsz = *(HEADER*)&buffer[head];
	q_b->frbufsz += (HEADERSZ + ROUND_BUFFER_SIZE(msgsz));

	head += HEADERSZ;
	
	if (head >= q_b->bufsz) {
		head = 0;
	}

	if ((remsz = q_b->bufsz - head) < msgsz) {
		raw_memcpy(msg, &buffer[head], remsz);
		msg = (RAW_U8 *)msg + remsz;
		msgsz -= remsz;
		head = 0;
	}
	raw_memcpy(msg, &buffer[head], msgsz);
	head += ROUND_BUFFER_SIZE(msgsz);
	if (head >= q_b->bufsz) {
		head = 0;
	}

	q_b->head = head;

	return actsz;
}

RAW_U16 queue_buffer_post(RAW_QUEUE_BUFFER *q_b, RAW_VOID *p_void, MSG_SIZE_TYPE msg_size, RAW_U8 opt_send_method)
{

	LIST *block_list_head;
	RAW_TASK_OBJ *task_ptr;

 	RAW_SR_ALLOC();
	
	RAW_CRITICAL_ENTER();

	if (q_b->common_block_obj.object_type != RAW_QUEUE_BUFFER_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &q_b->common_block_obj.block_list;
	
	if (!is_queue_buffer_free(q_b,  msg_size)) {

		RAW_CRITICAL_EXIT();

		TRACE_QUEUE_BUFFER_MAX(raw_task_active, q_b, p_void, msg_size, opt_send_method); 
		
		return RAW_QUEUE_BUFFER_FULL;
	}


	/*Queue buffer is not full here, if there is no blocked receive task*/
	if (is_list_empty(block_list_head)) {        

		if (opt_send_method == SEND_TO_END)  { 
			msg_to_end_buffer(q_b, p_void, msg_size);
		}

		else {


		}
		
		RAW_CRITICAL_EXIT();

		TRACE_QUEUE_BUFFER_POST(raw_task_active, q_b, p_void, msg_size, opt_send_method);
		
		return RAW_SUCCESS;
	}
	
	task_ptr = raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list);
	
	raw_memcpy(task_ptr->msg, p_void, msg_size);
	task_ptr->qb_msg_size = msg_size;
	
	raw_wake_object(task_ptr);
		
	RAW_CRITICAL_EXIT();

	TRACE_QUEUE_BUFFER_WAKE_TASK(raw_task_active, raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list), p_void, msg_size, opt_send_method);

	raw_sched();    
	return RAW_SUCCESS;
	

}



/*
************************************************************************************************************************
*                                    Post a msg to the queue buffer
*
* Description: This function is called to post a msg to the end of the queue buffer and inplemented fifo.
*
* Arguments  :p_q is the address of the queue object
*                 -----
*                  p_void  is the address of the message.
*                  if you want to use the extension memcpy, make sure the address is 4 bytes aligned.
*                 -----
*                 msg_size is the message size.                
*				         
* Returns			
*		RAW_SUCCESS: raw os return success
*            RAW_EXCEED_QUEUE_BUFFER_MSG_SIZE: message size exceed the max defined message size.
*		RAW_QUEUE_BUFFER_FULL:queue is full 
*            RAW_NOT_CALLED_BY_ISR: not supported in interrupt when CONFIG_RAW_ZERO_INTERRUPT is enabled.
*		
*		
*		
* Note(s)  This api is not supported in interrupt when CONFIG_RAW_ZERO_INTERRUPT is enabled.
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_buffer_end_post(RAW_QUEUE_BUFFER *q_b, RAW_VOID *p_void, MSG_SIZE_TYPE msg_size)
{


	#if (RAW_QUEUE_BUFFER_FUNCTION_CHECK > 0)

	if (q_b == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (p_void == 0) {
		
		return RAW_NULL_POINTER;
		
	}

	if (msg_size > q_b->maxmsz) {

		return RAW_EXCEED_QUEUE_BUFFER_MSG_SIZE;
	}
	
	#endif
	
	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif

	return queue_buffer_post(q_b, p_void, msg_size, SEND_TO_END);

}



/*
************************************************************************************************************************
*                                    Receive  a msg
*
* Description: This function is called to receive a msg
*
* Arguments  :q_b is the address of the queue buffer object
*                 -----
*                  msg is the address of a point, and it will be filled data lwithin this api.
*                  if you want to use the extension memcpy, make sure the msg address is 4 bytes aligned.
*                  -----	    
*                  wait_option: is  how the service behaves if the msg queue is full.
*							The wait options are
*							defined as follows:
*							RAW_NO_WAIT (0x00000000)
*							RAW_WAIT_FOREVER (0xFFFFFFFF)
*							timeout value (0x00000001
*							through
*							0xFFFFFFFE)
*                  receive_size:is the msg size received.
*				         
* Returns			
*						RAW_SUCCESS: raw os return success
*						RAW_BLOCK_DEL: if this queue is deleted.
*						RAW_BLOCK_TIMEOUT: queue is still full during waiting time when sending msg.
*						RAW_BLOCK_ABORT:queue is aborted during waiting time when sending msg.
*						RAW_STATE_UNKNOWN: possibly system error.
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_queue_buffer_receive(RAW_QUEUE_BUFFER *q_b, RAW_TICK_TYPE wait_option, RAW_VOID *msg, MSG_SIZE_TYPE *receive_size)
{
	RAW_U16 result;
	
	RAW_SR_ALLOC();


	#if (RAW_QUEUE_BUFFER_FUNCTION_CHECK > 0)

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}

	if (q_b == 0) {
		
		return RAW_NULL_OBJECT;
	}

	
	if (msg == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	#endif

	RAW_CRITICAL_ENTER();

	if (q_b->common_block_obj.object_type != RAW_QUEUE_BUFFER_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

  	if (!is_buffer_empty(q_b)) {
		
		*receive_size = buffer_to_msg(q_b, msg);
		
		RAW_CRITICAL_EXIT();

		return RAW_SUCCESS;   

  	}	
		

	if (wait_option == RAW_NO_WAIT) {   
	
		RAW_CRITICAL_EXIT();
		return RAW_NO_PEND_WAIT;
	} 

	SYSTEM_LOCK_PROCESS();

	raw_task_active->msg = msg;
	raw_pend_object((RAW_COMMON_BLOCK_OBJECT  *)q_b, raw_task_active, wait_option);
	
	RAW_CRITICAL_EXIT();

	raw_sched();

	result = block_state_post_process(raw_task_active, 0);
	
	/*if get the msg successful then take it*/
	if (result == RAW_SUCCESS) {

		*receive_size = raw_task_active->qb_msg_size;
	}
	
	return result;


}



/*
************************************************************************************************************************
*                                      Flush a queue buffer
*
* Description: This service deletes all messages stored in the specified message queue buffer.
*						
*			If the queue is empty, this service does nothing.
*
* Arguments  :p_q is the address of the queue buffer object 
*                    -----
*
* Returns			
*		     RAW_SUCCESS: raw os return success
*
*
* Note(s)   	
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_QUEUE_BUFFER_FLUSH > 0) 

RAW_U16 raw_queue_buffer_flush(RAW_QUEUE_BUFFER  *q_b)
{

	RAW_SR_ALLOC();

	#if (RAW_QUEUE_BUFFER_FUNCTION_CHECK > 0)

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}

	if (q_b == 0) {
		
		return RAW_NULL_OBJECT;
	}

	#endif

	RAW_CRITICAL_ENTER();

	if (q_b->common_block_obj.object_type != RAW_QUEUE_BUFFER_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	q_b->frbufsz = q_b->bufsz;
	q_b->head = 0;
	q_b->tail = 0;

	RAW_CRITICAL_EXIT();

	return RAW_SUCCESS;   
	
}

#endif


/*
************************************************************************************************************************
*                                      Delete a queue buffer
*
* Description: This function is called to delete a queue buffer.
*
* Arguments  :p_q is the address of this queue buffer object
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
#if (CONFIG_RAW_QUEUE_BUFFER_DELETE > 0)

RAW_U16 raw_queue_buffer_delete(RAW_QUEUE_BUFFER *q_b)
{
	LIST  *block_list_head;
	
	RAW_SR_ALLOC();

	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (q_b == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();

	if (q_b->common_block_obj.object_type != RAW_QUEUE_BUFFER_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &q_b->common_block_obj.block_list;
	
	q_b->common_block_obj.object_type = 0;
	
	/*All task blocked on this queue is waken up*/
	while (!is_list_empty(block_list_head))  {
		delete_pend_obj(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list));	
	}                             
	
	RAW_CRITICAL_EXIT();

	raw_sched(); 
	
	return RAW_SUCCESS;
	
}

#endif


/*
************************************************************************************************************************
*                                     Get queue buffer information
*
* Description: This function is called to get information form specified queue buffer.
*
* Arguments  :q_b is the address of this queue buffer object
*                 -----
*                  queue_buffer_free_size  is the pointer of free size of the queue buffer, which will be filled within this api. 
*                 -----
*                 queue_buffer_size is s the pointer of total size of the queue buffer, which will be filled within this api. 
*
*				         
* Returns			
*			RAW_SUCCESS: raw os return success
* Note(s)    	Commonly for debug purpose
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_QUEUE_BUFFER_GET_INFORMATION > 0)

RAW_U16 raw_queue_buffer_get_information(RAW_QUEUE_BUFFER  *q_b, RAW_U32 *queue_buffer_free_size, RAW_U32 *queue_buffer_size)
{
	RAW_SR_ALLOC();
	
	#if (RAW_QUEUE_BUFFER_FUNCTION_CHECK > 0)

	if (q_b == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (queue_buffer_free_size == 0) {
		
		return RAW_NULL_OBJECT;
	}

	if (queue_buffer_size == 0) {
		
		return RAW_NULL_OBJECT;
	}

	#endif

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif

	RAW_CRITICAL_ENTER();
	
	if (q_b->common_block_obj.object_type != RAW_QUEUE_BUFFER_OBJ_TYPE) {

		RAW_CRITICAL_EXIT();
		return RAW_ERROR_OBJECT_TYPE;
	}

	*queue_buffer_free_size = q_b->frbufsz;
	
	*queue_buffer_size = q_b->bufsz;

	RAW_CRITICAL_EXIT();
	
	return RAW_SUCCESS;

}

#endif


#endif


