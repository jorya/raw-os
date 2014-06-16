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

#ifndef RAW_INTERNAL_H
#define RAW_INTERNAL_H

void     raw_sched (void);
void     tick_list_init(void);
void     add_ready_list(RAW_RUN_QUEUE *rq, RAW_TASK_OBJ *task_ptr);
void     add_ready_list_head(RAW_RUN_QUEUE *rq, RAW_TASK_OBJ *task_ptr);
void     add_ready_list_end(RAW_RUN_QUEUE *rq, RAW_TASK_OBJ *task_ptr);
void     remove_ready_list(RAW_RUN_QUEUE *rq, RAW_TASK_OBJ *task_ptr);
void     move_to_ready_list_end(RAW_RUN_QUEUE *rq, RAW_TASK_OBJ *task_ptr);
void     calculate_time_slice(RAW_U8 task_prio);
void     get_ready_task(RAW_RUN_QUEUE *rq);

void     call_timer_task(void);
void     raw_timer_init(void);

void     change_pend_list_priority(RAW_TASK_OBJ *task_ptr);

RAW_U16  raw_wake_object(RAW_TASK_OBJ *task_ptr);
void     tick_list_remove(RAW_TASK_OBJ *task_ptr);
void     tick_list_insert(RAW_TASK_OBJ *task_ptr, RAW_TICK_TYPE time);
                 
RAW_U16  wake_send_msg(RAW_TASK_OBJ *task_ptr, RAW_VOID *msg);
RAW_U16  wake_send_msg_size(RAW_TASK_OBJ *task_ptr, RAW_VOID *msg, RAW_U32 msg_size);

RAW_U16  raw_pend_object(RAW_COMMON_BLOCK_OBJECT  *block_common_obj, RAW_TASK_OBJ *task_ptr, RAW_TICK_TYPE timeout);
RAW_U16  delete_pend_obj(RAW_TASK_OBJ *task_ptr);
RAW_VOID raw_idle_task (void *p_arg);

void     tick_list_update(void);
RAW_U16  block_state_post_process(RAW_TASK_OBJ  *task_ptr, RAW_VOID  **msg);

void     run_queue_init(RAW_RUN_QUEUE *rq);

RAW_S32  bit_search_first_one(RAW_U32 *base, RAW_U8 offset,  RAW_S32 width);
RAW_U8   chg_pri_mutex(RAW_TASK_OBJ *tcb, RAW_U8 priority, RAW_U16 *error);
RAW_U16  change_internal_task_priority(RAW_TASK_OBJ *task_ptr, RAW_U8 new_priority);

RAW_VOID mtx_chg_pri(RAW_TASK_OBJ *tcb, RAW_U8 oldpri);
RAW_VOID raw_task_free_mutex(RAW_TASK_OBJ *tcb);
RAW_VOID mutex_state_change(RAW_TASK_OBJ *tcb);
void     tick_task_start(void);
void     sche_disable_measure_start(void);
void     sche_disable_measure_stop(void);



extern RAW_TASK_OBJ                 *high_ready_obj;
extern RAW_TASK_OBJ                 *raw_task_active;

extern RAW_U8                       raw_os_active;
extern RAW_U8                       idle_task_exit;
extern RAW_U8                       task_0_exit;

extern RAW_RUN_QUEUE                raw_ready_queue;
extern RAW_U8                       raw_int_nesting;
extern RAW_U8                       raw_sched_lock;
extern RAW_TICK_TYPE                raw_tick_count;

extern RAW_TASK_OBJ                 raw_idle_obj;
extern RAW_IDLE_COUNT_TYPE          raw_idle_count;
extern PORT_STACK                   idle_stack[IDLE_STACK_SIZE];


extern LIST                         tick_head[TICK_HEAD_ARRAY];

#if (CONFIG_RAW_TIMER > 0)
extern LIST                         timer_head[TIMER_HEAD_NUMBERS];
extern RAW_TICK_TYPE                raw_timer_count;
extern RAW_U32                      raw_timer_ctrl;
extern RAW_TASK_OBJ                 raw_timer_obj;
extern PORT_STACK                   timer_task_stack[TIMER_STACK_SIZE];
extern RAW_SEMAPHORE                timer_sem;
extern RAW_MUTEX                    timer_mutex;
#endif

#if (CONFIG_RAW_TASK_0 > 0)
extern RAW_U16                      task_0_event_head;
extern RAW_U16                      task_0_event_end;
extern RAW_U16                      task_0_events;
extern RAW_U16                      peak_events;
extern EVENT_STRUCT                 task_0_events_queue[MAX_TASK_EVENT];
extern RAW_TASK_OBJ                 raw_task_0_obj;
extern PORT_STACK                   task_0_stack[TASK_0_STACK_SIZE];
extern EVENT_HANLDER                task_0_event_handler;

#if (CONFIG_RAW_ZERO_INTERRUPT > 0)
extern OBJECT_INT_MSG               object_int_msg[OBJECT_INT_MSG_SIZE];
extern OBJECT_INT_MSG               *free_object_int_msg;
extern RAW_U32                      int_msg_full;
extern EVENT_HANLDER                msg_event_handler;
#endif

#endif

extern RAW_OBJECT_DEBUG             raw_task_debug;

#if (CONFIG_RAW_MUTEX > 0)

extern RAW_U8                       mutex_recursion_levels;
extern RAW_U8                       mutex_recursion_max_levels;

#endif

#if (RAW_SCHE_LOCK_MEASURE_CHECK > 0)

extern PORT_TIMER_TYPE              raw_sche_disable_time_start;
extern PORT_TIMER_TYPE              raw_sche_disable_time_max;

#endif

#if (RAW_CONFIG_CPU_TIME > 0)

extern PORT_TIMER_TYPE              system_meaure_overhead;

#endif

#if (RAW_CONFIG_CPU_TASK > 0)

extern RAW_TASK_OBJ                 raw_cpu_obj;
extern PORT_STACK                   cpu_task_stack[CPU_STACK_SIZE];
extern RAW_IDLE_COUNT_TYPE          raw_idle_count_max;
extern RAW_U32                      cpu_usuage;
extern RAW_U32                      cpu_usuage_max;

#endif


#if (CONFIG_RAW_TICK_TASK > 0)
extern RAW_TASK_OBJ                 tick_task_obj;
extern PORT_STACK                   tick_task_stack[TICK_TASK_STACK_SIZE];
extern RAW_SEMAPHORE                tick_semaphore_obj;
#endif


#if (CONFIG_RAW_IDLE_EVENT > 0)

/*public global event*/
extern STATE_EVENT                  STM_GLOBAL_EVENT[4];

extern ACTIVE_EVENT_STRUCT_CB       active_idle_task[];

extern const RAW_U8                 raw_idle_map_table[256];

extern RAW_U8                       raw_idle_rdy_grp;                     
extern RAW_U8                       raw_rdy_tbl[8];
extern LIST                         raw_idle_tick_head;

#endif

#if (RAW_CPU_INT_DIS_MEASURE_CHECK > 0)

extern RAW_U16                      int_disable_times;
extern PORT_TIMER_TYPE              raw_int_disable_time_start;
extern PORT_TIMER_TYPE              raw_int_disable_time_max;

#endif


#endif

