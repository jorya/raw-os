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

#ifndef RAW_QUEUE_SIZE_H
#define RAW_QUEUE_SIZE_H


typedef struct RAW_MSG_SIZE {

	struct  RAW_MSG_SIZE        *next;							
	void                        *msg_ptr; 						 
	MSG_SIZE_TYPE                msg_size;
	
} RAW_MSG_SIZE;




typedef struct RAW_QUEUE_SIZE
{ 
	RAW_COMMON_BLOCK_OBJECT       common_block_obj;

	RAW_MSG_SIZE                  *free_msg;                           
	MSG_SIZE_TYPE                 number_free;  
	MSG_SIZE_TYPE                 peak_numbers;  /* Peak number of entries in the queue */
	                      
	RAW_MSG_SIZE                  *write;                            
	RAW_MSG_SIZE                  *read;                            
	MSG_SIZE_TYPE                 queue_msg_size;                   
	MSG_SIZE_TYPE                 queue_current_msg;   
		
} RAW_QUEUE_SIZE;


RAW_U16 raw_queue_size_create(RAW_QUEUE_SIZE  *p_q, RAW_U8 *p_name, RAW_MSG_SIZE *msg_start, MSG_SIZE_TYPE number);
RAW_U16 raw_queue_size_receive (RAW_QUEUE_SIZE *p_q, RAW_TICK_TYPE wait_option, RAW_VOID  **msg_ptr, MSG_SIZE_TYPE *receive_size);
RAW_U16 raw_queue_size_front_post(RAW_QUEUE_SIZE *p_q, RAW_VOID  *p_void, MSG_SIZE_TYPE size);
RAW_U16 raw_queue_size_end_post(RAW_QUEUE_SIZE *p_q, RAW_VOID  *p_void, MSG_SIZE_TYPE size);
RAW_U16 raw_queue_size_all_post(RAW_QUEUE_SIZE *p_q, RAW_VOID  *p_void, MSG_SIZE_TYPE size, RAW_U8 opt);
RAW_U16 raw_queue_size_full_check(RAW_QUEUE_SIZE *p_q);
RAW_U16 msg_size_post(RAW_QUEUE_SIZE *p_q, RAW_MSG_SIZE *p_void,  MSG_SIZE_TYPE size,  RAW_U8 opt_send_method, RAW_U8 opt_wake_all);             


#if (CONFIG_RAW_QUEUE_SIZE_FLUSH > 0) 
RAW_U16 raw_queue_size_flush(RAW_QUEUE_SIZE  *p_q);
#endif


#if (CONFIG_RAW_QUEUE_SIZE_DELETE > 0)
RAW_U16 raw_queue_size_delete(RAW_QUEUE_SIZE *p_q);
#endif

#if (CONFIG_RAW_QUEUE_SIZE_GET_INFORMATION > 0)
RAW_U16 raw_queue_size_get_information(RAW_QUEUE_SIZE *p_q, MSG_SIZE_TYPE *queue_free_msg_size, MSG_SIZE_TYPE *queue_peak_msg_size, MSG_SIZE_TYPE *queue_current_msg);
#endif


#endif


