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
  

#ifndef RAW_TASK_H
#define RAW_TASK_H

typedef  RAW_VOID    (*RAW_TASK_ENTRY)(RAW_VOID *p_arg);

RAW_U16 raw_task_create(RAW_TASK_OBJ  *task_obj, RAW_U8  *task_name,  RAW_VOID   *task_arg, 
                             RAW_U8  task_prio,  RAW_U32  time_slice,  PORT_STACK  *task_stack_base, 
                             RAW_U32 stack_size, RAW_TASK_ENTRY task_entry, RAW_U8 auto_start);


RAW_U16 raw_disable_sche(void);

RAW_U16 raw_enable_sche(void);

RAW_U16 raw_sleep(RAW_TICK_TYPE dly);
RAW_U16 raw_time_sleep(RAW_U16 hours, RAW_U16 minutes, RAW_U16 seconds, RAW_U32 milli);

#if (CONFIG_RAW_TASK_SUSPEND > 0)
RAW_U16 raw_task_suspend(RAW_TASK_OBJ *task_ptr);
RAW_U16 raw_task_resume(RAW_TASK_OBJ *task_ptr);
RAW_U16 task_suspend(RAW_TASK_OBJ *task_ptr);
RAW_U16 task_resume(RAW_TASK_OBJ *task_ptr);

#endif

#if (CONFIG_RAW_TASK_PRIORITY_CHANGE > 0)
RAW_U16 raw_task_priority_change (RAW_TASK_OBJ *task_ptr, RAW_U8 new_priority, RAW_U8 *old_priority);
#endif

#if (CONFIG_RAW_TASK_DELETE > 0)
RAW_U16 raw_task_delete(RAW_TASK_OBJ *task_ptr);
#endif

#if (CONFIG_RAW_TASK_WAIT_ABORT > 0)
RAW_U16 raw_task_wait_abort(RAW_TASK_OBJ *task_ptr);
#endif

#if (CONFIG_SCHED_FIFO_RR > 0)
RAW_U16 raw_task_time_slice_change(RAW_TASK_OBJ *task_ptr, RAW_U32 new_time_slice);
RAW_U16 raw_set_sched_way(RAW_TASK_OBJ *task_ptr, RAW_U8 policy);
RAW_U16 raw_get_sched_way(RAW_TASK_OBJ *task_ptr, RAW_U8 *policy_ptr);
#endif

RAW_TASK_OBJ  *raw_task_identify(void);

#if (CONFIG_RAW_TASK_STACK_CHECK > 0)
RAW_U16 raw_task_stack_check(RAW_TASK_OBJ  *task_obj, RAW_U32 *free_stack);
#endif

#if (CONFIG_USER_DATA_POINTER > 0)
RAW_VOID raw_set_task_user_point(RAW_TASK_OBJ *task_ptr, RAW_VOID *user_point, RAW_U32 point_position);

RAW_VOID *raw_get_task_user_point(RAW_TASK_OBJ *task_ptr, RAW_U32 point_position);
#endif

#if (CONFIG_RAW_DEBUG > 0)
RAW_U16 raw_iter_block_task(LIST *object_head, RAW_VOID  (*debug_function)(RAW_TASK_OBJ *arg), RAW_U8 opt);
RAW_U32 raw_get_system_global_space(void);
#endif

#define RAW_TASK_AUTO_START         1
#define	RAW_TASK_DONT_START         0

#endif
