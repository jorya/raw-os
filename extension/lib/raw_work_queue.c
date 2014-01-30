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


/* 	2013-1  Created by jorya_txj
  *	xxxxxx   please added here
  */

#include <raw_api.h>
#include <mm/raw_malloc.h>
#include <raw_work_queue.h>


OBJECT_WORK_QUEUE_MSG *free_work_queue_msg;

static void work_queue_task(void *pa)
{
	RAW_U16 ret;
	OBJECT_WORK_QUEUE_MSG *msg_recv;
	WORK_QUEUE_STRUCT *wq;

	RAW_SR_ALLOC();
	
	wq = pa;

	while (1) {

		ret = raw_queue_receive (&wq->queue, RAW_WAIT_FOREVER, (void **)(&msg_recv));

		if (ret != RAW_SUCCESS) {
			
			RAW_ASSERT(0);

		}

		msg_recv->handler(msg_recv->arg, msg_recv->msg);

		RAW_CPU_DISABLE();

		msg_recv->next = free_work_queue_msg;
		free_work_queue_msg = msg_recv;

		RAW_CPU_ENABLE();


	}

}

/*
************************************************************************************************************************
*                                       Establish a work queue
*
* Description: This function is called to establish a work queue.
*
* Arguments  :wq is the address of the work queue object.
*                	 ---------
*                    work_task_priority is the work queue task priority
*			 ---------	         
*			work_queue_stack_size is the work queue task stack size
*                   ---------
*                   work_queue_stack_base is the base work queue task address
*                   ---------
*                   msg_start is the start address of the msg, work queue need msg space.
*                   ----------
*                   work_msg_size is the size of the msg
*
* Returns     RAW_SUCCESS:  raw os return success.
*                 OTHERS:Fail.
*
* Note(s)    :Do  not use this API in interrupt.
*
*             
************************************************************************************************************************
*/
RAW_U16 work_queue_create(WORK_QUEUE_STRUCT *wq, RAW_U8 work_task_priority, RAW_U32 work_queue_stack_size, 
								PORT_STACK *work_queue_stack_base, RAW_VOID **msg_start, RAW_U32 work_msg_size)
{
	RAW_U16 ret;
	
	ret = raw_queue_create(&wq->queue, "work_queue", msg_start, work_msg_size);

	if (ret != RAW_SUCCESS) {

		return ret;
	}
	
	ret = raw_task_create(&wq->work_queue_task_obj, (RAW_U8  *)"work_queue", wq,
	                         work_task_priority, 0, work_queue_stack_base, 
	                         work_queue_stack_size, work_queue_task, 1); 

	if (ret != RAW_SUCCESS) {

		return ret;
	}

	return RAW_SUCCESS;
}


/*
************************************************************************************************************************
*                                       Schedule the specific work queue
*
* Description: This function is called to schedule the specific work queue
*
* Arguments  :wq is the address of the work queue object
*                    -----
*                    arg is the argument passed to the handler
*	               -----
*                    msg is the message passed to the handler
*				         
* Returns   : RAW_SUCCESS
*                  RAW_WORK_QUEUE_MSG_MAX: need more work_queue_internal_msg.
*                  RAW_MSG_MAX:queue is full.
*
* Note(s)  :   This API can be called by interrupt or task.  
*
*             
************************************************************************************************************************
*/
RAW_U16 sche_work_queue(WORK_QUEUE_STRUCT *wq, RAW_U32 arg, void *msg, WORK_QUEUE_HANDLER handler)
{
	void *msg_data;
	RAW_U16 ret;

	RAW_SR_ALLOC();

	RAW_CPU_DISABLE();
	
	if (free_work_queue_msg == 0) {
		
		RAW_CPU_ENABLE();
		
		return RAW_WORK_QUEUE_MSG_MAX;
	}

	msg_data = free_work_queue_msg;

	free_work_queue_msg->arg = arg;
	free_work_queue_msg->msg = msg;
	free_work_queue_msg->handler = handler;
	
	free_work_queue_msg = free_work_queue_msg->next;

	RAW_CPU_ENABLE();
	
	ret = raw_queue_end_post(&wq->queue, msg_data);

	return ret;
	
}


/*
************************************************************************************************************************
*                                       Init work queue system
*
* Description: This function is called to init the work queue system
*
* Arguments  :NONE
*                	 -----
*                
*				         
*				         
* Returns  :NONE
*
* Note(s)    :This function should be called before using any work queue API.
*
*             
************************************************************************************************************************
*/
void global_work_queue_init(OBJECT_WORK_QUEUE_MSG *work_queue_msg, RAW_U32 size)
{
	OBJECT_WORK_QUEUE_MSG *p_msg1;
	OBJECT_WORK_QUEUE_MSG *p_msg2;
	
	free_work_queue_msg = work_queue_msg;
	
	/*init the free msg list*/
	p_msg1 = work_queue_msg;
	p_msg2 = work_queue_msg;
	p_msg2++;

	while (--size) { 

		p_msg1->next = p_msg2;

		p_msg1++;
		p_msg2++;
	}

	/*init  the last free msg*/ 
	p_msg1->next = 0;                      
	
}

