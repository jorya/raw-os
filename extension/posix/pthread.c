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


/* 	2012-8  Created by jorya_txj
  *	xxxxxx   please added here
  */


#include <raw_api.h>
#include <posix/pthread.h>
#include <posix/errno.h>
#include <mm/raw_page.h>
#include <mm/raw_malloc.h>


#define DEFAULT_STACK_ELEMENT (10 * 1024)

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void * (*start_routine)(void *), void *arg)
{
	pthread_struct *thread_struct;
	void *stack_base;

	attr = attr;
	thread_struct = raw_malloc(sizeof(pthread_struct));
	stack_base 	  = raw_malloc(4 * DEFAULT_STACK_ELEMENT);
	
	*thread = thread_struct;

	raw_task_create(&thread_struct->task_obj, (RAW_U8  *)"task1", arg,
	                30, 0, stack_base, DEFAULT_STACK_ELEMENT, (RAW_TASK_ENTRY)start_routine, RAW_TASK_AUTO_START); 

	raw_semaphore_create(&thread_struct->task_sem, "sem", 0);
	thread_struct->task_sem.common_block_obj.block_way = RAW_BLOCKED_WAY_FIFO;
	return 0;
}



int pthread_detach(pthread_t thread)
{
	RAW_TASK_OBJ *task;
	task = &thread->task_obj;

	if (task->task_state == RAW_DELETED) {
		

	}

	else {

		RAW_CRITICAL_ENTER();
		thread->attr.detachstate = PTHREAD_CREATE_DETACHED;
		RAW_CRITICAL_EXIT();
		
		raw_semaphore_delete(&thread->task_sem);
			
	}

	return 0;
}



int pthread_join(pthread_t thread, void **thread_return)
{
	RAW_TASK_OBJ *task;
	RAW_U16	ret;
	
	task = &thread->task_obj;


	if (task == raw_task_identify()) {
	
		return EDEADLK;
	}

	

	ret = raw_semaphore_get(&thread->task_sem, RAW_WAIT_FOREVER);
	
	if (ret == RAW_SUCCESS) {
		raw_task_delete(task);
		*thread_return = thread->ret;
	}
	
	else {
		
		return ESRCH;
	}
	
	return 0;
}




void pthread_exit(void *value)
{
	RAW_TASK_OBJ *task;
	pthread_t thread;
	
	task = raw_task_identify();
	thread = (pthread_t)task;
		
	if (thread->attr.detachstate == PTHREAD_CREATE_JOINABLE) {

		/* release the joinable pthread */
		raw_semaphore_put(&thread->task_sem);
		return;
	}

	raw_task_delete(task);
	raw_semaphore_delete(&thread->task_sem);
	
}


