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


int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	RAW_U16 ret;
	attr = attr;
	
	ret = raw_mutex_create(&mutex->mutex_lock, (RAW_U8 *)"mutex", RAW_MUTEX_INHERIT_POLICY, 0);

	if (ret != RAW_SUCCESS) {
		
		return EINVAL;
	}


	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	RAW_U16 ret;

	ret = raw_mutex_delete(&mutex->mutex_lock);
		
	 if (ret != RAW_SUCCESS) {
	 	
	 	return EINVAL;
	 }
	

	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	RAW_U16 ret;

	ret = raw_mutex_get(&mutex->mutex_lock, RAW_WAIT_FOREVER);

	if (ret != RAW_SUCCESS) {

		return EINVAL;
	}


	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	RAW_U16 ret;

	ret = raw_mutex_put(&mutex->mutex_lock);

	if (ret != RAW_SUCCESS) {

		return EINVAL;
	}


	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	RAW_U16 ret;

	ret = raw_mutex_get(&mutex->mutex_lock, RAW_NO_WAIT);

	if (ret == RAW_NO_PEND_WAIT) {
		return EBUSY;
	}

	if (ret != RAW_SUCCESS) {

		return EINVAL;
	}

	
	return 0;
}

