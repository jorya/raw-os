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

#ifndef RAW_SYSTEM_H
#define RAW_SYSTEM_H

#define RAW_OS_VERSION                              1101

#define RAW_OS_BUILD_TIME                           "2015/10/09"     

#define IDLE_PRIORITY                               (CONFIG_RAW_PRIO_MAX - 1)

#define RAW_FALSE                                   0u
#define RAW_TRUE                                    1u

#define RAW_BLOCKED_WAY_FIFO                        1u
#define RAW_BLOCKED_WAY_PRIO                        0u

#define SCHED_FIFO                                  1u
#define SCHED_RR                                    2u


#if (CONFIG_RAW_LITTLE_ENDIAN > 0)
#define RAW_BIT_SET_N(n) ((RAW_U8)0x01 << ((n) & 7))
#else
#define RAW_BIT_SET_N(n) ((RAW_U8)0x80 >> ((n) & 7))
#endif



/* *****************************************************************************
 * Below are all the possible error codes the os can generate
 * ***************************************************************************** */
typedef enum raw_os_error
{
   
	RAW_SUCCESS                          = 0,
	RAW_IDLE_EXIT,	
	RAW_TASK_0_EXIT,	
	RAW_SYSTEM_ERROR,
	RAW_OS_RUNNING,
	RAW_OS_STOPPED,    
	RAW_NO_PEND_WAIT,
	RAW_SCHED_DISABLE,
	RAW_BLOCK_ABORT,
	RAW_BLOCK_TIMEOUT,
	RAW_BLOCK_DEL                        = 10,
	RAW_MSG_MAX,
	RAW_MSG_EMPTY,
	RAW_SCHED_LOCKED,
	RAW_SCHED_INVALID,
	RAW_SCHED_OVERFLOW,
	RAW_STATE_UNKNOWN,
	RAW_SUSPENDED_COUNT_OVERFLOWED,
	HAS_NOT_SUSPENDED,
	RAW_BYOND_MAX_PRIORITY,
	RAW_SAME_PRIORITY                    = 20,
	RAW_SEMAPHORE_OVERFLOW,
	RAW_SEMAPHORE_TASK_WAITING,
	RAW_MUTEX_NOT_RELEASE_BY_OCCYPY,
	RAW_MUTEX_OWNER_NESTED,
	RAW_MUTEX_BOOSTED_MAX,
	RAW_EXCEED_CEILING_PRIORITY,
	RAW_CEILING_PRIORITY_NOT_ALLOWED,
	RAW_MUTEX_NO_POLICY,
	RAW_NO_MEMORY,
	RAW_OBJ_INVALIDATE_STATE             = 30,
	RAW_TIMER_HAS_ACTIVED,
	RAW_TIMER_HAS_NOT_ACTIVED,
	RAW_TIMER_STATE_INVALID,
	RAW_TIMER_INVALID_TICKS,
	RAW_NULL_POINTER,
	RAW_BYTE_ALIGN_ERROR,
	RAW_NULL_OBJECT,
	RAW_INVALID_SCHED_WAY,
	RAW_NOT_CALLED_BY_ISR,
	RAW_NOT_CALLED_BY_TASK               = 40,
	RAW_ZERO_NUMBER,
	RAW_HIGHEST_TASK_EXIT,
	RAW_CHANGE_PRIORITY_NOT_ALLOWED,
	RAW_MUTEX_HOLDER_PRIORITY_CHANGED,
	RAW_DELETE_TASK_NOT_ALLOWED,
	RAW_SUSPEND_TASK_NOT_ALLOWED,
	RAW_NO_THIS_OPTION,
	RAW_BLOCK_SIZE_ERROR,
	RAW_BYTE_SIZE_ERROR,
	RAW_POOL_SIZE_INVALID                = 50,
	RAW_INVALID_ALIGN,
	RAW_INVALID_POIN,
	RAW_QUEUE_FULL_OPT_ERROR,
	RAW_ERROR_OBJECT_TYPE,
	RAW_IDLE_EVENT_EXHAUSTED,
	RAW_INVALID_HOURS,
	RAW_INVALID_MINUTES,
	RAW_TIME_ZERO_SLEEP,                  
	RAW_QUEUE_BUFFER_SIZE_0,
	RAW_QUEUE_BUFFER_INVALID_SIZE       = 60,
	RAW_QUEUE_BUFFER_FULL,
	RAW_EXCEED_QUEUE_BUFFER_MSG_SIZE,
	RAW_EXCEED_INT_NESTED_LEVEL,
	RAW_IDLE_TICK_ADD_FAILED,
	RAW_IDLE_TICK_DELETE_FAILED,
	RAW_WORK_QUEUE_MSG_MAX,
	RAW_INT_MSG_HANDLER_ERROR,
	RAW_MUTEX_RECURSION_LEVELS_EXCEEDED,
	RAW_SYSTEM_CRITICAL_ERROR,
	RAW_QUEUE_CHECK_FULL                = 70,
	RAW_QUEUE_CHECK_NOT_FULL,
	RAW_QUEUE_SIZE_CHECK_FULL,
	RAW_QUEUE_SIZE_CHECK_NOT_FULL,
	RAW_MUTEX_NESTED_OVERFLOW,
	RAW_INVALID_TASK_STATE
	
} RAW_OS_ERROR;





typedef enum raw_block_state
{
	RAW_B_OK = 0,
	RAW_B_ABORT,
	RAW_B_TIMEOUT,
	RAW_B_DEL
   
} RAW_BLOCK_STATE;


#define RAW_NO_WAIT      RAW_INTERNAL_NO_WAIT   
#define RAW_WAIT_FOREVER RAW_INTERNAL_WAIT_FOREVER

#define SEND_TO_FRONT     0x1
#define SEND_TO_END       0x2

typedef enum raw_int_msg_type{
                                               
	RAW_TYPE_Q_FRONT        = 0,
	RAW_TYPE_Q_END,
	RAW_TYPE_Q_ALL,
	RAW_TYPE_Q_BUFFER_END,
	RAW_TYPE_SEM, 
	RAW_TYPE_SEM_ALL,
	RAW_TYPE_SUSPEND,
	RAW_TYPE_RESUME,
	RAW_TYPE_Q_SIZE_FRONT,
	RAW_TYPE_Q_SIZE_END,
	RAW_TYPE_Q_SIZE_ALL    = 10,
	RAW_TYPE_EVENT,
	RAW_TYPE_IDLE_END_EVENT_POST,
	RAW_TYPE_IDLE_FRONT_EVENT_POST
	
} RAW_INT_MSG_TYPE;


typedef enum raw_object_type
{
	RAW_OBJ_TYPE_NONE = 0,
	RAW_SEM_OBJ_TYPE,
	RAW_MUTEX_OBJ_TYPE,
	RAW_QUEUE_OBJ_TYPE,
	RAW_QUEUE_BUFFER_OBJ_TYPE,
	RAW_QUEUE_SIZE_OBJ_TYPE,
	RAW_BYTE_OBJ_TYPE,
	RAW_BLOCK_OBJ_TYPE,
	RAW_TIMER_OBJ_TYPE,
	RAW_EVENT_OBJ_TYPE,
	RAW_MQUEUE_OBJ_TYPE
	
} RAW_OBJECT_TYPE;

RAW_OS_ERROR raw_enter_interrupt(void);
void     raw_time_tick(void);
void     raw_finish_int(void);

RAW_OS_ERROR raw_os_init(void);
RAW_OS_ERROR raw_os_start(void);

void *raw_memset(void  *src, RAW_U8  byte, RAW_U32 count);
void *raw_memcpy(void *dest, const void *src, RAW_U32 count);

RAW_TICK_TYPE raw_system_time_get(void);
RAW_OS_ERROR raw_system_time_set(RAW_TICK_TYPE time);

#endif

