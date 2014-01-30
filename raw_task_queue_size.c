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


/* 	2013-2  Created by jorya_txj
  *	xxxxxx   please added here
  */


#include <raw_api.h>

#if (CONFIG_RAW_TASK_QUEUE_SIZE > 0) 

/*
************************************************************************************************************************
*                                       Create a task queue size msg.  
*
* Description: This function is called to create a queue.
*
* Arguments  :task_obj is the address of task object want to be initialized 
*                 -----
*                  name_ptr  is the queue size object name
*                 -----
*                  msg_start is the  start address of msg buffer.
*				      ------ 
*                  number is the number of msgs  of the queue.
* Returns			
*			RAW_SUCCESS: raw os return success
* Note(s)    	one task must map to one queue.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_qsize_create(RAW_TASK_OBJ *task_obj, RAW_QUEUE_SIZE *queue_size_obj, RAW_U8 *p_name, RAW_MSG_SIZE *msg_start, RAW_U32 number)
{
	
	task_obj->task_queue_size_obj = queue_size_obj;
	
	return raw_queue_size_create(task_obj->task_queue_size_obj, p_name, msg_start, number);
		
}



/*
************************************************************************************************************************
*                                    Receive  a msg with size
*
* Description: This function is called to receive a msg with size
*
* Arguments  : msg is the address a point, and this pointer contains address of the msg.
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
* Note(s)    	The current task get the msg from its own queue.if no msg received then msg will get null pointer(0).
*
*             
************************************************************************************************************************
*/

RAW_U16 raw_task_qsize_receive (RAW_TICK_TYPE wait_option, RAW_VOID  **msg_ptr, RAW_U32 *receive_size)
{

	return raw_queue_size_receive (raw_task_active->task_queue_size_obj, wait_option, msg_ptr, receive_size);
}

/*
************************************************************************************************************************
*                                    Post a msg to the task queue front with size
*
* Description: This function is called to post a msg to the queue front and implement LIFO.
*
* Arguments  :task_obj is the address of task object want to be posted to.
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
RAW_U16 raw_task_qsize_front_post(RAW_TASK_OBJ *task_obj, RAW_VOID  *p_void, RAW_U32 size)
{

	return raw_queue_size_front_post(task_obj->task_queue_size_obj, p_void, size);
}


/*
************************************************************************************************************************
*                                    Post a msg to the task queue end with size
*
* Description: This function is called to post a msg to the queue end and implement FIFO.
*
* Arguments  :task_obj is the address of task object want to be posted to.
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
RAW_U16 raw_task_qsize_end_post(RAW_TASK_OBJ *task_obj, RAW_VOID  *p_void, RAW_U32 size)
{

	return raw_queue_size_end_post(task_obj->task_queue_size_obj, p_void, size);
}


/*
************************************************************************************************************************
*                                      Flush a queue 
*
* Description: This service deletes all messages stored in the specified task message queue.
*						If the queue is full, messages of all suspended threads are discarded.
*						Each blocked task is resumed and  return RAW_BLOCK_ABORT
*                  indicates the message send was aborted bt flush. If the queue is empty, this
						service does nothing.
*
* Arguments  :task_obj is the address of task object whose queue size want to be flushed.
* Returns			
*		RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_qsize_flush(RAW_TASK_OBJ *task_obj)
{

	return raw_queue_size_flush(task_obj->task_queue_size_obj);
	
}

/*
************************************************************************************************************************
*                                      Delete a task queue size object
*
* Description: This function is called to delete a queue size object.
*
* Arguments  :task_obj is the address of task object whose queue size wany to be deleted.
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
RAW_U16 raw_task_qsize_delete(RAW_TASK_OBJ *task_obj)
{

	return raw_queue_size_delete(task_obj->task_queue_size_obj);
}

/*
************************************************************************************************************************
*                                     Get task queue size information
*
* Description: This function is called to get information form queue.
*
* Arguments  :task_obj is the address of task object whose queue size information wany to be getted.
*                -----
*                queue_free_msg_size will be filled with free numbers of task queue size msg       
*		     -----
*               queue_peak_msg_size will be filled with the max used numbers of task queue size msg.
*               -------
*               queue_current_msg will be filled with the current used numbers of task queue size msg.
* Returns:			
*               RAW_SUCCESS: raw os return success
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_task_qsize_get_information(RAW_TASK_OBJ *task_obj, RAW_U32 *queue_free_msg_size, RAW_U32 *queue_peak_msg_size, RAW_U32 *queue_current_msg)
{

	return raw_queue_size_get_information(task_obj->task_queue_size_obj, queue_free_msg_size, queue_peak_msg_size, queue_current_msg);
	
}

#endif


