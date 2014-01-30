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


#ifndef RAW_MQUEUE_H
#define RAW_MQUEUE_H


typedef  RAW_VOID    *(*USER_MALLOC)(RAW_U32 arg);
typedef  RAW_VOID    (*USER_FREE)(RAW_VOID *arg);


typedef struct RAW_MQUEUE_MSG {
	RAW_U32          m_type;          
	RAW_U32          m_ts;
	RAW_VOID         *msg;

} RAW_MQUEUE_MSG;



typedef struct RAW_MQUEUE { 
	
	RAW_COMMON_BLOCK_OBJECT           common_block_obj;
	RAW_U32                           mq_curmsgs;
	RAW_U32                           peak_numbers;  /* Peak number of entries in the queue */
	RAW_U32                           mq_maxmsg;
	RAW_VOID                          **messages;
	USER_MALLOC                       malloc_fun;
	USER_FREE                         free_fun;
	
} RAW_MQUEUE;

#define WAKE_ALL_MQUEUE            0x1
#define WAKE_ONE_MQUEUE            0x0


RAW_U16 raw_mq_init(RAW_MQUEUE *mqueue, RAW_U8 *name_ptr, USER_MALLOC malloc_fun, USER_FREE free_fun, RAW_VOID **msg_start, RAW_U32 msg_size);

RAW_U16 raw_mq_send(RAW_MQUEUE *mqueue, RAW_VOID *msg_ptr, RAW_U32 msg_len, RAW_U32 msg_prio);
RAW_U16 raw_mq_receive (RAW_MQUEUE *p_q, RAW_VOID  **p_void, RAW_U32 *msg_len, RAW_U32 *msg_prio, RAW_TICK_TYPE wait_option);

#if (CONFIG_RAW_MQUEUE_FLUSH > 0)

RAW_U16 raw_mqueue_flush(RAW_MQUEUE  *p_q);

#endif

#if (CONFIG_RAW_MQUEUE_DELETE > 0)

RAW_U16 raw_mqueue_delete(RAW_MQUEUE *p_q);

#endif

#if (CONFIG_RAW_MQUEUE_GET_INFORMATION > 0)

RAW_U16 raw_mqueue_get_information(RAW_MQUEUE *p_q, RAW_U32 *queue_peak_msg_size, RAW_U32 *mq_curmsgs, RAW_U32 *mq_maxmsg);
										 
#endif


#endif


