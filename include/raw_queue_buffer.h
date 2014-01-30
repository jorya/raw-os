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

#ifndef RAW_QUEUE_BUFFER_H
#define RAW_QUEUE_BUFFER_H


typedef struct RAW_QUEUE_BUFFER
{ 
	RAW_COMMON_BLOCK_OBJECT       common_block_obj;
	
	MSG_SIZE_TYPE                 bufsz;   /* Message buffer size */
	MSG_SIZE_TYPE                 maxmsz;  /* Maximum length of message */
	MSG_SIZE_TYPE	              frbufsz; /* Free buffer size */
	MSG_SIZE_TYPE	              head;	   /* First message store address */
	MSG_SIZE_TYPE	              tail;	   /* Next to the last message store address */
	RAW_VOID                      *buffer; /* Message buffer address */
	
} RAW_QUEUE_BUFFER;

/*
 * Message header format
 */
typedef MSG_SIZE_TYPE           HEADER;

#define HEADERSZ                (sizeof(HEADER))

#define ROUND_SIZE              (sizeof(HEADER))

#define ROUND_BUFFER_SIZE(sz)   (((sz) + (ROUND_SIZE - 1)) & ~(ROUND_SIZE - 1))


RAW_U16 raw_queue_buffer_create(RAW_QUEUE_BUFFER *q_b, RAW_U8 *p_name, RAW_VOID *msg_buffer, MSG_SIZE_TYPE buffer_size, MSG_SIZE_TYPE max_msg_size);
RAW_U16 queue_buffer_post(RAW_QUEUE_BUFFER *q_b, RAW_VOID *p_void, MSG_SIZE_TYPE msg_size, RAW_U8 opt_send_method);
RAW_U16 raw_queue_buffer_end_post(RAW_QUEUE_BUFFER *q_b, RAW_VOID *p_void, MSG_SIZE_TYPE msg_size);
RAW_U16 raw_queue_buffer_receive(RAW_QUEUE_BUFFER *q_b, RAW_TICK_TYPE wait_option, RAW_VOID *msg, MSG_SIZE_TYPE *receive_size);

#if (CONFIG_RAW_QUEUE_BUFFER_FLUSH > 0) 
RAW_U16 raw_queue_buffer_flush(RAW_QUEUE_BUFFER  *q_b);
#endif

#if (CONFIG_RAW_QUEUE_BUFFER_DELETE > 0)
RAW_U16 raw_queue_buffer_delete(RAW_QUEUE_BUFFER *q_b);
#endif

#if (CONFIG_RAW_QUEUE_BUFFER_GET_INFORMATION > 0)
RAW_U16 raw_queue_buffer_get_information(RAW_QUEUE_BUFFER  *q_b, RAW_U32 *queue_buffer_free_size, RAW_U32 *queue_buffer_size);
#endif

#endif

