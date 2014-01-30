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


/* 	2012-10  Created by jorya_txj
  *	xxxxxx   please added here
  */

#ifndef RAW_TASK_0_H
#define RAW_TASK_0_H

typedef struct EVENT_HANLDER {

	void  (*handle_event)(TASK_0_EVENT_TYPE ev, void *event_data);
	
} EVENT_HANLDER;


typedef struct EVENT_STRUCT {
	
	TASK_0_EVENT_TYPE ev;
	void *event_data;
	EVENT_HANLDER *p;
  
} EVENT_STRUCT;


typedef struct OBJECT_INT_MSG {
	
	RAW_U8                  type;
	RAW_U8                  opt;
	
	struct OBJECT_INT_MSG   *next;
	
	void                    *object;
	void                    *msg;                           
	MSG_SIZE_TYPE           msg_size;                          
	RAW_U32                 event_flags;                             
                            
} OBJECT_INT_MSG;


RAW_U16 task_0_tick_post(void);
RAW_U16 raw_task_0_post(EVENT_HANLDER *p, TASK_0_EVENT_TYPE ev, void *event_data);
RAW_U16 raw_task_0_front_post(EVENT_HANLDER *p, TASK_0_EVENT_TYPE ev, void *event_data);
void raw_task_0_init(void);
RAW_U16 int_msg_post(RAW_U8 type, void *p_obj, void *p_void, MSG_SIZE_TYPE msg_size, RAW_U32 flags, RAW_U8 opt);

void hybrid_int_process(void);

#endif

