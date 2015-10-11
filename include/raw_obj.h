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

#ifndef RAW_OBJECT_H
#define RAW_OBJECT_H

typedef enum raw_task_status
{
	RAW_RDY = 0,
	RAW_PEND,
	RAW_PEND_TIMEOUT,
	RAW_PEND_TIMEOUT_SUSPENDED,
	RAW_SUSPENDED,
	RAW_PEND_SUSPENDED,
	RAW_DLY,
	RAW_DLY_SUSPENDED,
	RAW_DELETED,
	RAW_INVALID_STATE
} RAW_TASK_STATUS;


typedef	struct raw_common_block_object {	
	
	LIST                      block_list;
	RAW_U8                    *name;
	RAW_U8                    block_way;
	RAW_OBJECT_TYPE           object_type;
	
} RAW_COMMON_BLOCK_OBJECT;


typedef struct raw_task_obj
{
	void                 *task_stack;

	#if (CONFIG_RAW_MPU_ENABLE > 0)
	RAW_MPU_SETTINGS         mpu_settings;
	#endif

	LIST                     task_list;
	
	#if (CONFIG_USER_DATA_POINTER > 0)
	/*for user data extension*/
	void                 *user_data_pointer[CONFIG_USER_DATA_POINTER];
	
	#endif

	#if (CONFIG_SCHED_FIFO_RR > 0)
	/*For task time slice*/
	RAW_U32                  time_slice;  
	RAW_U32                  time_total;
	RAW_U8                   sched_way;
	#endif

	/* Current running priority */
	RAW_U8                   priority;  
	/* Base priority */
	RAW_U8                   bpriority; 

	#if (CONFIG_RAW_TASK_SUSPEND > 0)
	RAW_U8                   suspend_count;
	#endif
	
	#if (CONFIG_RAW_MUTEX > 0)
	struct raw_mutex         *mtxlist;
	#endif

	LIST                     task_debug_list;
	
	RAW_U32                  stack_size;
	
	PORT_STACK               *task_stack_base;
	LIST                     tick_list;

	RAW_TICK_TYPE            tick_match;
	RAW_TICK_TYPE            tick_remain; 

	LIST                     *tick_head;

	void                     *msg;

	MSG_SIZE_TYPE            msg_size;

	#if (CONFIG_RAW_QUEUE_BUFFER > 0)
	MSG_SIZE_TYPE            qb_msg_size;
	#endif
	
	RAW_U8                   *task_name;

	RAW_TASK_STATUS          task_state; 
	RAW_BLOCK_STATE          block_status;

	/*Task block on mutex, queue, semphore, event*/
	RAW_COMMON_BLOCK_OBJECT  *block_obj; 

	#if (CONFIG_RAW_TASK_QUEUE_SIZE > 0)
	struct raw_queue_size    *task_queue_size_obj;
	#endif
	
	#if (CONFIG_RAW_TASK_SEMAPHORE > 0)
	struct raw_semaphore     *task_semaphore_obj;
	#endif
	
	#if (CONFIG_RAW_EVENT > 0)
	RAW_U8                    raw_suspend_option;
	RAW_U32                   raw_suspend_flags;
	void                      *raw_additional_suspend_info;
	#endif

	#if (CONFIG_RAW_SYSTEM_STATISTICS > 0)
	RAW_U32                    task_free_stack_size;
	RAW_SYS_TIME_TYPE          task_time_total_run;
	RAW_SYS_TIME_TYPE          task_time_total_run_prev;
	RAW_HARD_TIME_TYPE         task_time_start;
	RAW_HARD_TIME_TYPE         task_exec_time;
	#endif
		   
} RAW_TASK_OBJ;


#define  NUM_WORDS					((CONFIG_RAW_PRIO_MAX + 31) / 32)


typedef	struct raw_run_queue {

	RAW_U8                    highest_priority;
	
	LIST                      task_ready_list[CONFIG_RAW_PRIO_MAX];	
	
	RAW_U32                   task_bit_map[NUM_WORDS];
	
} RAW_RUN_QUEUE;
		

typedef struct raw_object_debug {
	
	/*Debug task head*/
	LIST                      task_head;
	
} RAW_OBJECT_DEBUG;

#endif

