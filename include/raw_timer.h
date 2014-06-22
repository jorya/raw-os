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

#ifndef RAW_TIMER_H
#define RAW_TIMER_H

typedef struct RAW_TIMER {

	LIST                                    timer_list;
	LIST                                    *to_head;

	RAW_U8                                  *name;    
	RAW_U16                                 (*raw_timeout_function)(RAW_VOID *arg);       

	RAW_TICK_TYPE                           match;                
	RAW_TICK_TYPE                           remain; 
	RAW_TICK_TYPE                           init_count;
	RAW_TICK_TYPE                           reschedule_ticks;
	RAW_VOID                                *raw_timeout_param;
	RAW_U8                                  timer_state;
	RAW_U8                                  object_type;
    
 }RAW_TIMER;


#define TIMER_DEACTIVE                      1
#define TIMER_ACTIVE                        2
#define TIMER_DELETED                       3
#define TIMER_CALLBACK_CONTINUE             4
#define TIMER_CALLBACK_STOP                 0x88


typedef struct RAW_TIMER_HEAD {
	struct  RAW_TIMER       *first_timer_ptr;      
}RAW_TIMER_HEAD;



RAW_U16 raw_timer_create(RAW_TIMER *timer_ptr, RAW_U8  *name_ptr,
                              RAW_U16  (*expiration_function)(RAW_VOID *expiration_input), RAW_VOID *expiration_input,
                              RAW_TICK_TYPE initial_ticks, RAW_TICK_TYPE reschedule_ticks, RAW_U8 auto_activate);


RAW_U16 raw_timer_activate(RAW_TIMER *timer_ptr, RAW_VOID *expiration_input);


#if (CONFIG_RAW_TIMER_DEACTIVATE > 0)
RAW_U16 raw_timer_deactivate(RAW_TIMER *timer_ptr);
#endif

#if (CONFIG_RAW_TIMER_DELETE > 0)
RAW_U16 raw_timer_delete(RAW_TIMER *timer_ptr);
#endif

#if (CONFIG_RAW_TIMER_CHANGE > 0)
RAW_U16 raw_timer_change(RAW_TIMER *timer_ptr, RAW_TICK_TYPE initial_ticks, RAW_TICK_TYPE reschedule_ticks);
#endif

#endif

