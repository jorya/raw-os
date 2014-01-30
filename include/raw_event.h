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

#ifndef RAW_EVENT_H
#define RAW_EVENT_H


typedef struct RAW_EVENT
{ 
	RAW_COMMON_BLOCK_OBJECT    common_block_obj;
	RAW_U32                    flags;
	
} RAW_EVENT;



#define RAW_FLAGS_AND_MASK                   0x2u
#define RAW_FLAGS_CLEAR_MASK                 0x1u


#define RAW_AND                              0x02u
#define RAW_AND_CLEAR                        0x03u
#define RAW_OR                               0x00u
#define RAW_OR_CLEAR                         0x01u


RAW_U16 raw_event_create(RAW_EVENT *event_ptr, RAW_U8 *name_ptr, RAW_U32 flags_init);
RAW_U16 raw_event_get(RAW_EVENT *event_ptr, RAW_U32 requested_flags, RAW_U8 get_option, RAW_U32 *actual_flags_ptr, RAW_TICK_TYPE wait_option);
RAW_U16 raw_event_set(RAW_EVENT *event_ptr, RAW_U32  flags_to_set, RAW_U8 set_option);
RAW_U16 event_set(RAW_EVENT *event_ptr, RAW_U32 flags_to_set, RAW_U8 set_option);

#if (CONFIG_RAW_EVENT_DELETE > 0)
RAW_U16 raw_event_delete(RAW_EVENT *event_ptr);
#endif

#endif

