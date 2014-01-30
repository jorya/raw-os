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


#ifndef RAW_QUEUE_H
#define RAW_QUEUE_H

typedef struct RAW_MSG_Q {    
	
	RAW_VOID         **queue_start;            /* Pointer to start of queue data                              */
	RAW_VOID         **queue_end;              /* Pointer to end   of queue data                              */
	RAW_VOID         **write;               /* Pointer to where next message will be inserted  in   the Q  */
	RAW_VOID         **read;              /* Pointer to where next message will be extracted from the Q  */
	MSG_SIZE_TYPE    size;             /* Size of queue (maximum number of entries)                   */
	MSG_SIZE_TYPE    current_numbers;          /* Current number of entries in the queue                      */
	MSG_SIZE_TYPE    peak_numbers;          /* Peak number of entries in the queue                      */
		
} RAW_MSG_Q;



typedef struct RAW_MSG_INFO { 
	
	RAW_MSG_Q      msg_q;
	
	LIST           *suspend_entry;
	
} RAW_MSG_INFO;


typedef struct RAW_QUEUE
{ 
	RAW_COMMON_BLOCK_OBJECT       common_block_obj;
	RAW_MSG_Q                     msg_q;
	RAW_VOID                      (*queue_send_notify)(struct RAW_QUEUE *queue_ptr);
	
} RAW_QUEUE;

typedef RAW_VOID (*QUEUE_SEND_NOTIFY)(RAW_QUEUE *queue_ptr);

#define WAKE_ALL_QUEUE    0x1
#define WAKE_ONE_QUEUE    0x0

RAW_U16 raw_queue_create(RAW_QUEUE  *p_q, RAW_U8    *p_name, RAW_VOID **msg_start, MSG_SIZE_TYPE number);
RAW_U16 raw_queue_front_post(RAW_QUEUE *p_q, RAW_VOID  *p_void);
RAW_U16 raw_queue_end_post(RAW_QUEUE *p_q, RAW_VOID  *p_void);
RAW_U16 raw_queue_receive (RAW_QUEUE *p_q, RAW_TICK_TYPE wait_option, RAW_VOID  **msg);
RAW_U16 raw_queue_all_post(RAW_QUEUE *p_q, RAW_VOID  *p_void, RAW_U8 opt);
RAW_U16 raw_queue_send_notify(RAW_QUEUE *p_q, QUEUE_SEND_NOTIFY notify_function);
RAW_U16 raw_queue_post_notify(RAW_QUEUE *p_q, RAW_VOID *p_void);
RAW_U16 raw_queue_full_check(RAW_QUEUE *p_q);
RAW_U16 msg_post(RAW_QUEUE *p_q, RAW_VOID *p_void, RAW_U8 opt_send_method, RAW_U8 opt_wake_all);

#if (CONFIG_RAW_QUEUE_FLUSH > 0) 
RAW_U16 raw_queue_flush(RAW_QUEUE  *p_q);
#endif

#if (CONFIG_RAW_QUEUE_DELETE > 0)
RAW_U16 raw_queue_delete(RAW_QUEUE *p_q);
#endif

#if (CONFIG_RAW_QUEUE_GET_INFORMATION > 0)
RAW_U16 raw_queue_get_information(RAW_QUEUE *p_q, RAW_MSG_INFO *msg_information);
#endif



#endif

