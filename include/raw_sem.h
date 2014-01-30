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

#ifndef RAW_SEM_H
#define RAW_SEM_H

typedef struct RAW_SEMAPHORE
{ 
	RAW_COMMON_BLOCK_OBJECT       common_block_obj;
	RAW_U32                       count;
	RAW_VOID                      (*semphore_send_notify)(struct RAW_SEMAPHORE *queue_ptr);	
	
} RAW_SEMAPHORE;


typedef RAW_VOID (*SEMPHORE_SEND_NOTIFY)(RAW_SEMAPHORE *queue_ptr);

#define RAW_SEMAPHORE_COUNT   0xffffffff
#define WAKE_ALL_SEM          0x1
#define WAKE_ONE_SEM          0x0

RAW_U16 raw_semaphore_create(RAW_SEMAPHORE *semaphore_ptr, RAW_U8 *name_ptr, RAW_U32 initial_count);
RAW_U16 raw_semaphore_put(RAW_SEMAPHORE *semaphore_ptr);
RAW_U16 raw_semaphore_put_all(RAW_SEMAPHORE *semaphore_ptr);
RAW_U16 raw_semphore_send_notify(RAW_SEMAPHORE *semaphore_ptr, SEMPHORE_SEND_NOTIFY notify_function);
RAW_U16 raw_semaphore_put_notify(RAW_SEMAPHORE *semaphore_ptr);
RAW_U16 raw_semaphore_get(RAW_SEMAPHORE *semaphore_ptr, RAW_TICK_TYPE wait_option);
RAW_U16 semaphore_put(RAW_SEMAPHORE *semaphore_ptr, RAW_U8 opt_wake_all);

#if (CONFIG_RAW_SEMAPHORE_SET > 0)
RAW_U16 raw_semaphore_set(RAW_SEMAPHORE *semaphore_ptr,  RAW_U32 sem_count);
#endif

#if (CONFIG_RAW_SEMAPHORE_DELETE > 0)
RAW_U16 raw_semaphore_delete(RAW_SEMAPHORE *semaphore_ptr);
#endif


#endif
