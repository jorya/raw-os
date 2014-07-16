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

#if (CONFIG_RAW_MQUEUE > 0)


/*
************************************************************************************************************************
*                                       Create a mqueue 
*
* Description: This function is called to create a mqueue.
*
* Arguments  :mqueue is the address of queue object want to be initialized 
*                 -----
*                  name_ptr  is the queue object name
*                 -----
*                  malloc_fun is the malloc function for mqueue internal use, block memory allocated method is recommended.
*		      ------ 
*                  free_fun is the free function for mqueue internal use, block memory free method is recommended.
* Returns			
*		      RAW_SUCCESS: raw os return success
* Note(s)    	mqueue is not used in interupt, it is only used for task communication!
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_mq_init(RAW_MQUEUE *mqueue, RAW_U8 *name_ptr, USER_MALLOC malloc_fun, USER_FREE free_fun, RAW_VOID **msg_start, RAW_U32 msg_size)
{

	mqueue->malloc_fun = malloc_fun;
	mqueue->free_fun = free_fun;
	mqueue->messages = msg_start;
	mqueue->mq_maxmsg = msg_size;
	
	mqueue->mq_curmsgs = 0u;
	mqueue->peak_numbers = 0u;

	/*Init the list*/
	list_init(&mqueue->common_block_obj.block_list);
	mqueue->common_block_obj.block_way = RAW_BLOCKED_WAY_PRIO;
	mqueue->common_block_obj.name = name_ptr; 
	mqueue->common_block_obj.object_type = RAW_MQUEUE_OBJ_TYPE;
	return RAW_SUCCESS;
}


/* Auxiliary functions to manipulate messages' list */
RAW_INLINE void msg_insert(RAW_MQUEUE *info, RAW_MQUEUE_MSG *ptr)
{
	RAW_S32 k;

	k = info->mq_curmsgs - 1u;
	while ((k >= 0) && ( ( ( RAW_MQUEUE_MSG * )info->messages[k]  )->m_type >= ptr->m_type  )  ) {
		info->messages[k + 1] = info->messages[k];
		k--;
	}
	info->mq_curmsgs++;

	if (info->mq_curmsgs > info->peak_numbers) {
		
		info->peak_numbers = info->mq_curmsgs;
	}

	info->messages[k + 1] = ptr;
}

static RAW_U16 internal_raw_mq_send(RAW_MQUEUE *p_q, RAW_VOID *p_void, RAW_U32 size, RAW_U32 msg_prio)
{
	
	LIST *block_list_head;
	RAW_MQUEUE_MSG *sended_block_msg;
	
	RAW_SR_ALLOC();

	#if (RAW_QUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	#endif

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
		
	}

	if (p_q->common_block_obj.object_type !=  RAW_MQUEUE_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &p_q->common_block_obj.block_list;
	sended_block_msg = p_q->malloc_fun(sizeof(RAW_MQUEUE_MSG));

	if (sended_block_msg == 0) {

		return RAW_NO_MEMORY;

	}
	sended_block_msg->msg = p_void;
	sended_block_msg->m_ts = size;
	sended_block_msg->m_type = msg_prio;
	
	raw_disable_sche();

	if (p_q->mq_curmsgs >= p_q->mq_maxmsg) {
		
		raw_enable_sche();
		p_q->free_fun(sended_block_msg);
		return RAW_MSG_MAX;
		
	}


	
	/*Queue is not full here, there should be no blocked send task*/	
	/*If there is no blocked receive task*/
	if (is_list_empty(block_list_head)) { 
		
		msg_insert(p_q, sended_block_msg);
		raw_enable_sche();
		return RAW_SUCCESS;
	}

	RAW_CRITICAL_ENTER();
	
	/*wake hignhest priority task blocked on this queue and send msg to it*/
	wake_send_msg(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list), sended_block_msg);	
		
	RAW_CRITICAL_EXIT();

	raw_enable_sche();
	
   	raw_sched();
	
	return RAW_SUCCESS;

}

/*
************************************************************************************************************************
*                                    Post a priority msg to the mqueue
*
* Description: This function is called to post a msg to the queue end and implement FIFO.
*
* Arguments  :mqueue is the address of the mqueue object
*                 -----
*                 msg_ptr  is the address of the msg
*                 -----
*                 msg_len is the length of the msg
*                -------
*                msg_prio is the priority of the msg, big number means high priority
*		    -------
* Returns			
*			RAW_SUCCESS: raw os return success
*			RAW_MSG_MAX:mqueue is full 
*			
*			
*			
* Note(s)    mqueue is not used in interupt, it is only used for task communication!	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_mq_send(RAW_MQUEUE *mqueue, RAW_VOID *msg_ptr, RAW_U32 msg_len, RAW_U32 msg_prio)
{
	RAW_U16 ret;
	
	ret = internal_raw_mq_send(mqueue, msg_ptr, msg_len, msg_prio);
	return ret;

}



/*
************************************************************************************************************************
*                                    Receive a msg from mqueue
*
* Description: This function is called to receive a msg from mqueue
*
* Arguments  :p_q is the address of the mqueue object
*                 -----
*                  p_void is the address of a point, and this pointer contains address of the msg.
*                  -----	    
*			msg_len will be filled with msg lenght later
*			-----
*			msg_prio will be filled with msg_prio later
*			-----
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
* Note(s)    	if no msg received then msg will get null pointer(0).
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_mq_receive (RAW_MQUEUE *p_q,  RAW_VOID **p_void, RAW_U32 *msg_len, RAW_U32 *msg_prio, RAW_TICK_TYPE wait_option)
{
	RAW_U16 result;
	RAW_MQUEUE_MSG *msg_in;
	
	RAW_SR_ALLOC();

	#if (RAW_MQUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}

	
	if (p_void == 0) {
		
		return RAW_NULL_POINTER;
	}
	
	#endif

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;	
	}

	if (p_q->common_block_obj.object_type !=  RAW_MQUEUE_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}

	raw_disable_sche();
	
	/*if queue has msg then receive it*/
	if (p_q->mq_curmsgs) {

		msg_in = p_q->messages[p_q->mq_curmsgs - 1u];
		*p_void =  msg_in->msg;
		*msg_len = msg_in->m_ts;
		*msg_prio = msg_in->m_type;
		p_q->mq_curmsgs--;
		
		raw_enable_sche();

		p_q->free_fun(msg_in);
		
		return RAW_SUCCESS;
	
	}


	if (wait_option == RAW_NO_WAIT) {

		*p_void =  0;
		*msg_len = 0u;
		*msg_prio = 0u;
		
		raw_enable_sche();
		return RAW_NO_PEND_WAIT;
	} 

	if (raw_sched_lock >= 2) { 
		
		*p_void =  0;
		*msg_len = 0u;
		*msg_prio = 0u;
		
		raw_enable_sche();
		return RAW_SCHED_DISABLE;    
	}

	RAW_CRITICAL_ENTER();
	raw_pend_object((RAW_COMMON_BLOCK_OBJECT  *)p_q, raw_task_active, wait_option);
	RAW_CRITICAL_EXIT();
	
	raw_enable_sche();

	raw_sched(); 
	
	result = block_state_post_process(raw_task_active, 0);

	/*if get the msg successful then take it*/
	if (result == RAW_SUCCESS) {

		msg_in = raw_task_active->msg;
		*p_void =  msg_in->msg;
		*msg_len = msg_in->m_ts;
		*msg_prio = msg_in->m_type;
		p_q->free_fun(msg_in);
	}

	else {
		
		*p_void =  0;
		*msg_len = 0u;
		*msg_prio = 0u;

	}
	
	return result;
	
}


/*
************************************************************************************************************************
*                                      Flush a mqueue 
*
* Description: This service deletes all messages stored in the specified message queue.
*						If the queue is full, messages of all suspended threads are discarded.
*						Each blocked task is resumed and  return RAW_BLOCK_ABORT
*                  indicates the message send was aborted bt flush. If the queue is empty, this
						service does nothing.
*
* Arguments  :p_q is the address of the queue object 
*                 -----
*                 msg  is the address of the msg want to be posted when queue is flushed.
*                 -----
* Returns			
*			RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_MQUEUE_FLUSH > 0)

RAW_U16 raw_mqueue_flush(RAW_MQUEUE  *p_q)
{

	RAW_SR_ALLOC();
	
	#if (RAW_MQUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	#endif

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;	
	}

	if (p_q->common_block_obj.object_type !=  RAW_MQUEUE_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}
	
	RAW_CRITICAL_ENTER();
	p_q->mq_curmsgs = 0u;
	RAW_CRITICAL_EXIT(); 
	
	return RAW_SUCCESS;
}

#endif



/*
************************************************************************************************************************
*                                      Delete a mqueue 
*
* Description: This function is called to delete a mqueue.
*
* Arguments  :p_q is the address of this mqueue object
*                 -----
*                  All blocked task will be waked up and receive a RAW_BLOCK_DEL
*				         
* Returns			
*		      RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_MQUEUE_DELETE > 0)
RAW_U16 raw_mqueue_delete(RAW_MQUEUE *p_q)
{
	LIST  *block_list_head;
	
	RAW_SR_ALLOC();

	#if (RAW_MQUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0) {
		
		return RAW_NULL_OBJECT;
	}
	
	#endif

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;	
	}

	if (p_q->common_block_obj.object_type !=  RAW_MQUEUE_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}
	

	block_list_head = &p_q->common_block_obj.block_list;
	
	RAW_CRITICAL_ENTER();

	p_q->common_block_obj.object_type = 0u;
	
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
*                                     Get mqueue information
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
* Note(s)    	
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_MQUEUE_GET_INFORMATION > 0)
RAW_U16 raw_mqueue_get_information(RAW_MQUEUE *p_q, RAW_U32 *queue_peak_msg_size, RAW_U32 *mq_curmsgs, RAW_U32 *mq_maxmsg)
{

	RAW_SR_ALLOC();
	
	#if (RAW_MQUEUE_FUNCTION_CHECK > 0)

	if (p_q == 0)  {
		
		return RAW_NULL_OBJECT;
	}

	
	#endif


	
	if (p_q->common_block_obj.object_type !=  RAW_MQUEUE_OBJ_TYPE) {

		return RAW_ERROR_OBJECT_TYPE;
	}
	
	RAW_CPU_DISABLE();
	
	*mq_curmsgs = p_q->mq_curmsgs;
	*mq_maxmsg = p_q->mq_maxmsg;
	*queue_peak_msg_size = p_q->peak_numbers;
	
	RAW_CPU_ENABLE();

	return RAW_SUCCESS;

}

#endif

#endif

