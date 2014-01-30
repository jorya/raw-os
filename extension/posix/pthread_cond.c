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




int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
	RAW_U16 ret;
	attr = attr;
	
	ret = raw_semaphore_create(&cond->sem_lock,"cond", 0);
	
	if (ret != RAW_SUCCESS) {
		
		return EINVAL;
	}

	cond->attr = -1;
	return 0;
}


int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{

	if (cond->attr != -1) {

		return EINVAL;

	}
	
	if (raw_task_active != mutex->mutex_lock.mtxtsk) {
		
		return EINVAL;
	}
	
	
	if (raw_mutex_put(&mutex->mutex_lock) != RAW_SUCCESS) {
		
		return EINVAL;
	}

	
	raw_semaphore_get(&cond->sem_lock, RAW_WAIT_FOREVER);

	
	raw_mutex_get(&mutex->mutex_lock, RAW_WAIT_FOREVER);


	return 0;
	
}



int pthread_cond_destroy(pthread_cond_t *cond)
{

	
	RAW_U16 ret;

	if (cond->attr != -1) {

		return EINVAL;

	}

	cond->attr = 0;
	
	ret = raw_semaphore_delete(&cond->sem_lock);
	
	if (ret != RAW_SUCCESS) {
		return EBUSY;
	}
	
	
	return 0;

}


int pthread_cond_broadcast(pthread_cond_t *cond)
{

	if (cond->attr != -1) {

		return EINVAL;

	}
	
	if (raw_semaphore_put_all(&cond->sem_lock) != RAW_SUCCESS) {
		
		return EINVAL;
	}
	
	return 0;
}


int pthread_cond_signal(pthread_cond_t *cond)
{
	if (cond->attr != -1) {

		return EINVAL;

	}
	
	if (raw_semaphore_put(&cond->sem_lock) != RAW_SUCCESS) {
		
		return EINVAL;
	}

	return 0;
}



int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t * mutex, const struct timespec *abstime)
{

	RAW_U16 ret;
	RAW_U32 ticks;

	if (cond->attr != -1) {

		return EINVAL;

	}
	
	
	if (raw_task_active != mutex->mutex_lock.mtxtsk) {
		
		return EINVAL;
	}
	
	
	if (raw_mutex_put(&mutex->mutex_lock) != RAW_SUCCESS) {
		
		return EINVAL;
	}

	ticks = calculate_ticks(abstime);
	ret = raw_semaphore_get(&cond->sem_lock, ticks);

	
	raw_mutex_get(&mutex->mutex_lock, RAW_WAIT_FOREVER);

	if (ret == RAW_BLOCK_TIMEOUT) {

		return ETIMEDOUT;

	}

	if (ret != RAW_SUCCESS) {
		return EINVAL;
	}

	return 0;

}


