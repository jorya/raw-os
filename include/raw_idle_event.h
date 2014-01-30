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

/* 	2012-12  Created by jorya_txj
  *	xxxxxx   please added here
  */


#ifndef RAW_IDLE_EVENT_H
#define RAW_IDLE_EVENT_H

typedef struct IDLE_QUEUE_MSG {

	RAW_U16 sig;
	void *para;
	
} IDLE_QUEUE_MSG;


typedef struct ACTIVE_EVENT_STRUCT {

	STM_STRUCT super;                
	
	RAW_TICK_TYPE tick_ctr;
	
	LIST   idle_tick_list;
		
	RAW_U8 head;

	RAW_U8 prio;
	
	RAW_U8 tail;
	
	RAW_U8 nUsed;
	
	RAW_U8 priority_bit_x;
	RAW_U8 priority_bit_y;

	RAW_U8 priority_x;
	RAW_U8 priority_y;

} ACTIVE_EVENT_STRUCT;


typedef struct ACTIVE_EVENT_STRUCT_CB {
    ACTIVE_EVENT_STRUCT *act;        
    IDLE_QUEUE_MSG    *queue;          
    RAW_U8 end;                 
	
} ACTIVE_EVENT_STRUCT_CB;


#define  ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define  RAW_IDLE_LOWEST_PRIO 63u
#define  RAW_IDLE_RDY_TBL_SIZE ((RAW_IDLE_LOWEST_PRIO) / 8u + 1u)

void idle_event_init(void);
RAW_U16 event_post(ACTIVE_EVENT_STRUCT *me, RAW_U16 sig, void *para, RAW_U8 opt_send_method);
RAW_U16 idle_event_end_post(ACTIVE_EVENT_STRUCT *me, RAW_U16 sig, void *para);
RAW_U16 idle_event_front_post(ACTIVE_EVENT_STRUCT *me, RAW_U16 sig, void *para);
void idle_tick_isr(void);
RAW_U16 idle_tick_arm(ACTIVE_EVENT_STRUCT *me, RAW_TICK_TYPE ticks);
RAW_U16 idle_tick_disarm(ACTIVE_EVENT_STRUCT *me);
void idle_run(void); 
void idle_event_user(void);

#endif

