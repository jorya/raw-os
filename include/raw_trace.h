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


/* 	2013-4  Created by jorya_txj
  *	xxxxxx   please added here
  */


#ifndef RAW_TRACE_H
#define RAW_TRACE_H

#if (CONFIG_RAW_TRACE_ENABLE > 0)

void _trace_init(void);
void _trace_task_switch(RAW_TASK_OBJ *from, RAW_TASK_OBJ *to);
void _trace_int_task_switch(RAW_TASK_OBJ *from, RAW_TASK_OBJ *to);
void _trace_task_create(RAW_TASK_OBJ *task);
void _trace_task_priority_change(RAW_TASK_OBJ *task, RAW_U8 priority);
void _trace_task_suspend(RAW_TASK_OBJ *task_obj, RAW_TASK_OBJ *task);
void _trace_task_resume(RAW_TASK_OBJ *task_obj, RAW_TASK_OBJ *task_resumed);
void _trace_task_delete(RAW_TASK_OBJ *task);
void _trace_task_abort(RAW_TASK_OBJ *task);
void _trace_task_0_overflow(EVENT_HANLDER *p, TASK_0_EVENT_TYPE ev, void *event_data);
void _trace_int_msg_exhausted(void);
void _trace_int_msg_post(RAW_U8 type, void *p_obj, void *p_void, RAW_U32 msg_size, RAW_U32 flags, RAW_U8 opt);

void _trace_semaphore_create(RAW_TASK_OBJ *task, RAW_SEMAPHORE *semaphore_obj);
void _trace_semaphore_overflow(RAW_TASK_OBJ *task, RAW_SEMAPHORE *semaphore_obj);

void _trace_semaphore_delete(RAW_TASK_OBJ *task, RAW_SEMAPHORE *semaphore_obj);

void _trace_semaphore_get_success(RAW_TASK_OBJ *task, RAW_SEMAPHORE *semaphore_obj);

void _trace_semaphore_get_block(RAW_TASK_OBJ *task, RAW_SEMAPHORE *semaphore_obj, RAW_TICK_TYPE wait_option);

void _trace_sem_wake_task(RAW_TASK_OBJ *task, RAW_TASK_OBJ *task_waked_up, RAW_U8 opt_wake_all);

void _trace_semaphore_count_increase(RAW_TASK_OBJ *task, RAW_SEMAPHORE *semaphore_obj);

void _trace_queue_create(RAW_TASK_OBJ *task, RAW_QUEUE *queue_obj);

void _trace_queue_msg_post(RAW_TASK_OBJ *task, RAW_QUEUE *queue_obj, void *msg, RAW_U8 opt_send_method);

void _trace_queue_wake_task(RAW_TASK_OBJ *task, RAW_TASK_OBJ *task_waked_up, void *msg, RAW_U8 opt_wake_all);

void _trace_queue_msg_max(RAW_TASK_OBJ *task, RAW_QUEUE *queue_obj, void *msg, RAW_U8 opt_send_method);

void _trace_queue_get_msg(RAW_TASK_OBJ *task, RAW_QUEUE *queue_obj, RAW_TICK_TYPE wait_option, void *msg);

void _trace_queue_get_block(RAW_TASK_OBJ *task, RAW_QUEUE *queue_obj, RAW_TICK_TYPE wait_option);

void _trace_queue_flush(RAW_TASK_OBJ *task, RAW_QUEUE *queue_obj);

void _trace_queue_delete(RAW_TASK_OBJ *task, RAW_QUEUE *queue_obj);

void _trace_mutex_create(RAW_TASK_OBJ *task, RAW_MUTEX *mutex_ptr, RAW_U8 *name_ptr, RAW_U8 policy, RAW_U8 ceiling_prio);

void _trace_mutex_release(RAW_TASK_OBJ *task, RAW_TASK_OBJ *tcb, RAW_U8 newpri);

void _trace_mutex_ex_ce_pri(RAW_TASK_OBJ *task, RAW_MUTEX *mutex_obj, RAW_TICK_TYPE wait_option);

void _trace_mutex_get(RAW_TASK_OBJ *task, RAW_MUTEX *mutex_obj, RAW_TICK_TYPE wait_option);

void _trace_task_pri_inv(RAW_TASK_OBJ *task, RAW_TASK_OBJ *mtxtsk);

void _trace_mutex_get_block(RAW_TASK_OBJ *task, RAW_MUTEX *mutex_obj, RAW_TICK_TYPE wait_option);

void _trace_mutex_release_success(RAW_TASK_OBJ *task, RAW_MUTEX *mutex_obj);

void _trace_mutex_wake_task(RAW_TASK_OBJ *task, RAW_TASK_OBJ *task_waked_up);

void _trace_mutex_delete(RAW_TASK_OBJ *task, RAW_MUTEX *mutex_obj);

void _trace_task_stack_space(RAW_TASK_OBJ *task_obj);

void _trace_block_no_memory(RAW_TASK_OBJ *task_obj, MEM_POOL *pool_obj);

void _trace_block_pool_create(RAW_TASK_OBJ *task_obj, MEM_POOL *pool_obj);

void _trace_byte_no_memory(RAW_TASK_OBJ *task_obj, RAW_BYTE_POOL_STRUCT *pool_obj);

void _trace_event_create(RAW_TASK_OBJ *task_obj, RAW_EVENT *event_obj, RAW_U8 *name, RAW_U32 flags_init);

void _trace_event_get(RAW_TASK_OBJ *task_obj, RAW_EVENT *event_obj);

void _trace_event_get_block(RAW_TASK_OBJ *task_obj, RAW_EVENT *event_obj, RAW_TICK_TYPE wait_option);

void _trace_event_wake(RAW_TASK_OBJ *task_obj, RAW_TASK_OBJ *task);

void _trace_event_delete(RAW_TASK_OBJ *task_obj, RAW_EVENT *event_obj);

void _trace_queue_size_create(RAW_TASK_OBJ *task_obj, RAW_QUEUE_SIZE *queue_size_obj);

void _trace_queue_size_msg_max(RAW_TASK_OBJ *task_obj, RAW_QUEUE_SIZE *queue_size_obj, void *msg, MSG_SIZE_TYPE msg_size, RAW_U8 opt_send_method);

void _trace_queue_size_msg_post(RAW_TASK_OBJ *task_obj, RAW_QUEUE_SIZE *queue_size_obj, void *msg, MSG_SIZE_TYPE msg_size, RAW_U8 opt_send_method);

void _trace_queue_size_wake_task(RAW_TASK_OBJ *task_obj, RAW_TASK_OBJ *task_waked_up, void *msg, MSG_SIZE_TYPE msg_size, RAW_U8 opt_wake_all);

void _trace_queue_size_get_msg(RAW_TASK_OBJ *task_obj, RAW_QUEUE_SIZE *queue_size_obj, RAW_TICK_TYPE wait_option, void *msg, RAW_U32 msg_size);

void _trace_queue_size_get_block(RAW_TASK_OBJ *task_obj, RAW_QUEUE_SIZE *queue_size_obj, RAW_TICK_TYPE wait_option);

void _trace_queue_size_flush(RAW_TASK_OBJ *task_obj, RAW_QUEUE_SIZE *queue_size_obj);

void _trace_queue_size_delete(RAW_TASK_OBJ *task_obj, RAW_QUEUE_SIZE *queue_size_obj);

void _trace_queue_buffer_create(RAW_TASK_OBJ *task_obj, RAW_QUEUE_BUFFER *queue_buffer_obj);

void _trace_buffer_max(RAW_TASK_OBJ *task_obj, RAW_QUEUE_BUFFER *queue_buffer_obj, RAW_VOID *p_void, MSG_SIZE_TYPE msg_size, RAW_U8 opt_send_method);

void _trace_buffer_post(RAW_TASK_OBJ *task_obj, RAW_QUEUE_BUFFER *queue_buffer_obj, RAW_VOID *p_void, MSG_SIZE_TYPE msg_size, RAW_U8 opt_send_method);

void _trace_queue_buffer_wake_task(RAW_TASK_OBJ *task_obj, RAW_TASK_OBJ *task_waked_up, RAW_VOID *p_void, MSG_SIZE_TYPE msg_size, RAW_U8 opt_send_method);

void _trace_queue_buffer_get_block(RAW_TASK_OBJ *task_obj, RAW_QUEUE_BUFFER *queue_buffer_obj, RAW_TICK_TYPE wait_option);

void _trace_queue_fp_time_record(RAW_QUEUE *p_q, RAW_VOID *p_void);

void _trace_queue_ep_time_record(RAW_QUEUE *p_q, RAW_VOID *p_void);

void _trace_queue_ap_time_record(RAW_QUEUE *p_q, RAW_VOID *p_void, RAW_U8 opt);

void _trace_int_msg_handle_error(TASK_0_EVENT_TYPE ev, RAW_U16 int_msg_ret);


#define TRACE_INIT()  _trace_init()

#define TRACE_TASK_SWITCH(from, to)  _trace_task_switch(from, to)

#define TRACE_INT_TASK_SWITCH(from, to) _trace_int_task_switch(from, to)

#define TRACE_TASK_CREATE(task) _trace_task_create(task)

#define TRACE_TASK_PRIORITY_CHANGE(task, priority) _trace_task_priority_change(task, priority)

#define TRACE_TASK_SUSPEND(task_obj, task) _trace_task_suspend(task_obj, task)

#define TRACE_TASK_RESUME(task_obj, task_resumed) _trace_task_resume(task_obj, task_resumed)

#define TRACE_TASK_DELETE(task) _trace_task_delete(task)

#define TRACE_TASK_WAIT_ABORT(task) _trace_task_abort(task)

#define TRACE_TASK_0_OVERFLOW(p, ev, event_data) _trace_task_0_overflow(p, ev, event_data)

#define TRACE_INT_MSG_EXHAUSTED() _trace_int_msg_exhausted()

#define TRACE_INT_MSG_POST(type, p_obj, p_void, msg_size, flags, opt) _trace_int_msg_post(type, p_obj, p_void, msg_size, flags, opt)

#define TRACE_SEMAPHORE_CREATE(task, semaphore_obj) _trace_semaphore_create(task, semaphore_obj)

#define TRACE_SEMAPHORE_OVERFLOW(task, semaphore_obj) _trace_semaphore_overflow(task, semaphore_obj)

#define TRACE_SEMAPHORE_COUNT_INCREASE(task, semaphore_obj) _trace_semaphore_count_increase(task, semaphore_obj)

#define TRACE_SEMAPHORE_GET_SUCCESS(task, semaphore) _trace_semaphore_get_success(task, semaphore)

#define TRACE_SEMAPHORE_GET_BLOCK(task, semaphore, wait_option) _trace_semaphore_get_block(task, semaphore, wait_option)

#define TRACE_SEM_WAKE_TASK(task, task_waked_up, opt_wake_all) _trace_sem_wake_task(task, task_waked_up, opt_wake_all)

#define TRACE_SEMAPHORE_DELETE(task, semaphore_obj) _trace_semaphore_delete(task, semaphore_obj)

#define TRACE_QUEUE_CREATE(task, queue) _trace_queue_create(task, queue)

#define TRACE_QUEUE_MSG_MAX(task, queue_obj, msg, opt_send_method) _trace_queue_msg_max(task, queue_obj, msg, opt_send_method) 

#define TRACE_QUEUE_MSG_POST(task, queue_obj, msg, opt_send_method) _trace_queue_msg_post(task, queue_obj, msg, opt_send_method)

#define TRACE_QUEUE_WAKE_TASK(task, task_waked_up, p_void, opt_wake_all) _trace_queue_wake_task(task, task_waked_up, p_void, opt_wake_all)

#define TRACE_QUEUE_GET_MSG(task, queue_obj, wait_option, msg) _trace_queue_get_msg(task, queue_obj, wait_option, msg)

#define TRACE_QUEUE_GET_BLOCK(task, queue_obj, wait_option) _trace_queue_get_block(task, queue_obj, wait_option)

#define TRACE_QUEUE_FLUSH(task, queue_obj) _trace_queue_flush(task, queue_obj)

#define TRACE_QUEUE_DELETE(task, queue_obj) _trace_queue_delete(task, queue_obj)

#define TRACE_MUTEX_CREATE(task, mutex_obj, name, policy, ceiling_prio) _trace_mutex_create(task, mutex_obj, name, policy, ceiling_prio)

#define TRACE_MUTEX_RELEASE(task, tcb, newpri) _trace_mutex_release(task, tcb, newpri)

#define TRACE_MUTEX_EX_CE_PRI(task, mutex_obj, wait_option) _trace_mutex_ex_ce_pri(task, mutex_obj, wait_option)

#define TRACE_MUTEX_GET(task, mutex_obj, wait_option) _trace_mutex_get(task, mutex_obj, wait_option)

#define TRACE_TASK_PRI_INV(task, mtxtsk) _trace_task_pri_inv(task, mtxtsk)

#define TRACE_MUTEX_GET_BLOCK(task, mutex_obj, wait_option) _trace_mutex_get_block(task, mutex_obj, wait_option)

#define TRACE_MUTEX_RELEASE_SUCCESS(task, mutex_obj)  _trace_mutex_release_success(task, mutex_obj)

#define TRACE_MUTEX_WAKE_TASK(task, task_waked_up) _trace_mutex_wake_task(task, task_waked_up)

#define TRACE_MUTEX_DELETE(task, mutex_obj) _trace_mutex_delete(task, mutex_obj)

#define TRACE_TASK_STACK_SPACE(task_obj) _trace_task_stack_space(task_obj)

#define TRACE_BLOCK_NO_MEMORY(task_obj, pool_obj) _trace_block_no_memory(task_obj, pool_obj)

#define TRACE_BLOCK_POOL_CREATE(task_obj, pool_obj) _trace_block_pool_create(task_obj, pool_obj)

#define TRACE_BYTE_NO_MEMORY(task_obj, pool_obj) _trace_byte_no_memory(task_obj, pool_obj)

#define TRACE_EVENT_CREATE(task_obj, event_obj, name, flags_init) _trace_event_create(task_obj, event_obj, name, flags_init)

#define TRACE_EVENT_GET(task_obj, event_obj) _trace_event_get(task_obj, event_obj)

#define TRACE_EVENT_GET_BLOCK(task_obj, event_obj, wait_option) _trace_event_get_block(task_obj, event_obj, wait_option)

#define TRACE_EVENT_WAKE(task_obj, task) _trace_event_wake(task_obj, task)

#define TRACE_EVENT_DELETE(task_obj, event_obj) _trace_event_delete(task_obj, event_obj)

#define TRACE_QUEUE_SIZE_CREATE(task_obj, queue_size_obj) _trace_queue_size_create(task_obj, queue_size_obj)

#define TRACE_QUEUE_SIZE_MSG_MAX(task_obj, queue_size_obj, msg, msg_size, opt_send_method) _trace_queue_size_msg_max(task_obj, queue_size_obj, msg, msg_size, opt_send_method)

#define TRACE_QUEUE_SIZE_MSG_POST(task_obj, queue_size_obj, msg, msg_size, opt_send_method) _trace_queue_size_msg_post(task_obj, queue_size_obj, msg, msg_size, opt_send_method)

#define TRACE_QUEUE_SIZE_WAKE_TASK(task_obj, task_waked_up, msg, msg_size, opt_wake_all) _trace_queue_size_wake_task(task_obj, task_waked_up, msg, msg_size, opt_wake_all)

#define TRACE_QUEUE_SIZE_GET_MSG(task_obj, queue_size_obj, wait_option, msg, msg_size) _trace_queue_size_get_msg(task_obj, queue_size_obj, wait_option, msg, msg_size)

#define TRACE_QUEUE_SIZE_GET_BLOCK(task_obj, queue_size_obj, wait_option) _trace_queue_size_get_block(task_obj, queue_size_obj, wait_option)

#define TRACE_QUEUE_SIZE_FLUSH(task_obj, queue_size_obj) _trace_queue_size_flush(task_obj, queue_size_obj)

#define TRACE_QUEUE_SIZE_DELETE(task_obj, queue_size_obj) _trace_queue_size_delete(task_obj, queue_size_obj)

#define TRACE_QUEUE_BUFFER_CREATE(task_obj, queue_buffer_obj) _trace_queue_buffer_create(task_obj, queue_buffer_obj)

#define TRACE_QUEUE_BUFFER_MAX(task_obj, queue_buffer_obj, msg, msg_size, opt_send_method) _trace_buffer_max(task_obj, queue_buffer_obj, msg, msg_size, opt_send_method)

#define TRACE_QUEUE_BUFFER_POST(task_obj, queue_buffer_obj, msg, msg_size, opt_send_method) _trace_buffer_post(task_obj, queue_buffer_obj, msg, msg_size, opt_send_method)

#define TRACE_QUEUE_BUFFER_WAKE_TASK(task_obj, task_waked_up, p_void, msg_size, opt_send_method) _trace_queue_buffer_wake_task(task_obj, task_waked_up, p_void, msg_size, opt_send_method)

#define TRACE_QUEUE_BUFFER_GET_BLOCK(task, queue_buffer_obj, wait_option) _trace_queue_buffer_get_block(task, queue_buffer_obj, wait_option)

#define TRACE_QUEUE_FP_TIME_RECORD(p_q, p_void)        _trace_queue_fp_time_record(p_q, p_void)

#define TRACE_QUEUE_EP_TIME_RECORD(p_q, p_void)        _trace_queue_ep_time_record(p_q, p_void)

#define TRACE_QUEUE_AP_TIME_RECORD(p_q, p_void, opt)   _trace_queue_ap_time_record(p_q, p_void, opt)

#define TRACE_INT_MSG_HANDLE_ERROR(ev, int_msg_ret)    _trace_int_msg_handle_error(ev, int_msg_ret)

#else

#define TRACE_INIT()

#define TRACE_TASK_SWITCH(from, to)

#define TRACE_INT_TASK_SWITCH(from, to)

#define TRACE_TASK_CREATE(task)

#define TRACE_TASK_PRIORITY_CHANGE(task, priority)

#define TRACE_TASK_SUSPEND(task_obj, task)

#define TRACE_TASK_RESUME(task, task_resumed)

#define TRACE_TASK_DELETE(task)

#define TRACE_TASK_WAIT_ABORT(task)

#define TRACE_TASK_0_OVERFLOW(p, ev, event_data)

#define TRACE_INT_MSG_EXHAUSTED()

#define TRACE_INT_MSG_POST(type, p_obj, p_void, msg_size, flags, opt)

#define TRACE_SEMAPHORE_CREATE(task, semaphore_obj)

#define TRACE_SEMAPHORE_OVERFLOW(task, semaphore_obj)

#define TRACE_SEMAPHORE_COUNT_INCREASE(task, semaphore_obj)

#define TRACE_SEMAPHORE_GET_SUCCESS(task, semaphore)

#define TRACE_SEMAPHORE_GET_BLOCK(task, semaphore, wait_option)

#define TRACE_SEM_WAKE_TASK(task, task_waked_up, opt_wake_all)

#define TRACE_SEMAPHORE_DELETE(task, semaphore_obj)

#define TRACE_QUEUE_CREATE(task, queue)

#define TRACE_QUEUE_MSG_MAX(task, queue_obj, msg, opt_send_method)

#define TRACE_QUEUE_MSG_POST(task, queue_obj, msg, opt_send_method)

#define TRACE_QUEUE_WAKE_TASK(task, task_waked_up, p_void, opt_wake_all)

#define TRACE_QUEUE_GET_MSG(task, queue_obj, wait_option, msg)

#define TRACE_QUEUE_GET_BLOCK(task, queue_obj, wait_option)

#define TRACE_QUEUE_FLUSH(task, queue_obj)

#define TRACE_QUEUE_DELETE(task, queue_obj)

#define TRACE_MUTEX_CREATE(task, mutex_obj, name, policy, ceiling_prio)

#define TRACE_MUTEX_RELEASE(task, tcb, newpri)

#define TRACE_MUTEX_EX_CE_PRI(task, mutex_obj, wait_option)

#define TRACE_MUTEX_GET(task, mutex_obj, wait_option)

#define TRACE_TASK_PRI_INV(task, mtxtsk)

#define TRACE_MUTEX_GET_BLOCK(task, mutex_obj, wait_option)

#define TRACE_MUTEX_RELEASE_SUCCESS(task, mutex_obj) 

#define TRACE_MUTEX_WAKE_TASK(task, task_waked_up)

#define TRACE_MUTEX_DELETE(task, mutex_obj)

#define TRACE_TASK_STACK_SPACE(task_obj)

#define TRACE_BLOCK_NO_MEMORY(task_obj, pool_obj)

#define TRACE_BLOCK_POOL_CREATE(task_obj, pool_obj)

#define TRACE_BYTE_NO_MEMORY(task_obj, pool_obj)

#define TRACE_EVENT_CREATE(task_obj, event_obj, name, flags_init)

#define TRACE_EVENT_GET(task_obj, event_obj)

#define TRACE_EVENT_GET_BLOCK(task_obj, event_obj, wait_option)

#define TRACE_EVENT_WAKE(task_obj, task)

#define TRACE_EVENT_DELETE(task_obj, event_obj)

#define TRACE_QUEUE_SIZE_CREATE(task_obj, queue_size_obj)

#define TRACE_QUEUE_SIZE_MSG_MAX(task_obj, queue_size_obj, msg, msg_size, opt_send_method)

#define TRACE_QUEUE_SIZE_MSG_POST(task_obj, queue_size_obj, msg, msg_size, opt_send_method)

#define TRACE_QUEUE_SIZE_WAKE_TASK(task_obj, task_waked_up, msg, msg_size, opt_wake_all)

#define TRACE_QUEUE_SIZE_GET_MSG(task_obj, queue_size_obj, wait_option, msg, msg_size)

#define TRACE_QUEUE_SIZE_GET_BLOCK(task_obj, queue_size_obj, wait_option)

#define TRACE_QUEUE_SIZE_FLUSH(task_obj, queue_size_obj)

#define TRACE_QUEUE_SIZE_DELETE(task_obj, queue_size_obj)

#define TRACE_QUEUE_BUFFER_CREATE(task_obj, queue_buffer_obj)

#define TRACE_QUEUE_BUFFER_MAX(task_obj, queue_buffer_obj, msg, msg_size, opt_send_method)

#define TRACE_QUEUE_BUFFER_POST(task_obj, queue_buffer_obj, msg, msg_size, opt_send_method)

#define TRACE_QUEUE_BUFFER_WAKE_TASK(task_obj, task_waked_up, p_void, msg_size, opt_send_method)

#define TRACE_QUEUE_BUFFER_GET_BLOCK(task, queue_obj, wait_option)

#define TRACE_QUEUE_FP_TIME_RECORD(p_q, p_void)

#define TRACE_QUEUE_EP_TIME_RECORD(p_q, p_void)

#define TRACE_QUEUE_AP_TIME_RECORD(p_q, p_void, opt)

#define TRACE_INT_MSG_HANDLE_ERROR(ev, int_msg_ret)

#endif

#endif



