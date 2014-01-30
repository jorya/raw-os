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

#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#define RAW_WORK_QUEUE_MSG_MAX      0xff00

typedef RAW_VOID (*WORK_QUEUE_HANDLER)(RAW_U32 arg, void *msg);

typedef struct OBJECT_WORK_QUEUE_MSG {

	struct OBJECT_WORK_QUEUE_MSG   *next; 
	WORK_QUEUE_HANDLER      handler;
	void                    *msg;
	RAW_U32                 arg;
                                        
} OBJECT_WORK_QUEUE_MSG;


typedef struct WORK_QUEUE_STRUCT {

	RAW_QUEUE queue;
	RAW_TASK_OBJ work_queue_task_obj;

} WORK_QUEUE_STRUCT;



#endif


RAW_U16 work_queue_create(WORK_QUEUE_STRUCT *wq, RAW_U8 work_task_priority, RAW_U32 work_queue_stack_size, 
								PORT_STACK *work_queue_stack_base, RAW_VOID **msg_start, RAW_U32 work_msg_size);

RAW_U16 sche_work_queue(WORK_QUEUE_STRUCT *wq, RAW_U32 arg, void *msg, WORK_QUEUE_HANDLER handler);

void global_work_queue_init(OBJECT_WORK_QUEUE_MSG *work_queue_msg, RAW_U32 size);


