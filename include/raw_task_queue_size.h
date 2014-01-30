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

#ifndef RAW_TASK_QUEUE_SIZE_H
#define RAW_TASK_QUEUE_SIZE_H

RAW_U16 raw_task_qsize_create(RAW_TASK_OBJ *task_obj, RAW_QUEUE_SIZE *queue_size_obj, RAW_U8 *p_name, RAW_MSG_SIZE *msg_start, RAW_U32 number);

RAW_U16 raw_task_qsize_receive (RAW_TICK_TYPE wait_option, RAW_VOID  **msg_ptr, RAW_U32 *receive_size);

RAW_U16 raw_task_qsize_front_post(RAW_TASK_OBJ *task_obj, RAW_VOID  *p_void, RAW_U32 size);

RAW_U16 raw_task_qsize_end_post(RAW_TASK_OBJ *task_obj, RAW_VOID  *p_void, RAW_U32 size);

RAW_U16 raw_task_qsize_flush(RAW_TASK_OBJ *task_obj);

RAW_U16 raw_task_qsize_delete(RAW_TASK_OBJ *task_obj);

RAW_U16 raw_task_qsize_get_information(RAW_TASK_OBJ *task_obj, RAW_U32 *queue_free_msg_size, RAW_U32 *queue_peak_msg_size, RAW_U32 *queue_current_msg);

#endif

