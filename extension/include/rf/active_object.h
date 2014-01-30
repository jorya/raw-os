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


/* 	2012-9  Created by jorya_txj
  *	xxxxxx   please added here
  */


#ifndef ACTIVE_OBJECT_H

#define ACTIVE_OBJECT_H

typedef struct ACTIVE_OBJECT_STRUCT {
 	/*State machine*/  
    STM_STRUCT        father;

    RAW_QUEUE         active_queue;

    RAW_TASK_OBJ      thread;

	/*broadcast queue list*/
	LIST              active_queue_list[ACTIVE_MAX_BROADCAST_SIGNAL];

    RAW_U8            prio;
	RAW_U8            user_data;

} ACTIVE_OBJECT_STRUCT;


#endif


void active_event_post_end(ACTIVE_OBJECT_STRUCT *me, STATE_EVENT *event);
void active_event_post_front(ACTIVE_OBJECT_STRUCT *me, STATE_EVENT *event);
STATE_EVENT *active_event_get(ACTIVE_OBJECT_STRUCT *me);
void active_object_create(ACTIVE_OBJECT_STRUCT *me, RAW_U8 prio,
                   RAW_VOID **msg_start, RAW_U32 qLen,
                   void *stkSto, RAW_U32 stkSize,
                   STATE_EVENT *event);

void active_object_delete(ACTIVE_OBJECT_STRUCT *me);

void active_event_defer_post(RAW_QUEUE *q, STATE_EVENT *event);

RAW_U16 active_event_recall(ACTIVE_OBJECT_STRUCT *me, RAW_QUEUE *q);

