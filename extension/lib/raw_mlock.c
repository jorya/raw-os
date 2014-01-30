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


/* 	2012-10  Created by jorya_txj
  *	xxxxxx   please added here
  */

#include <raw_api.h>
#include <raw_mlock.h>

/*get mlock*/
RAW_U16 raw_mlock(RAW_MLOCK *mlock, RAW_U8 num, RAW_TICK_TYPE wait_option)
{
	RAW_U16 ret;
	RAW_U32 flag;

	ret = raw_event_get(&mlock->event_ptr, 1 << num, RAW_AND_CLEAR, &flag, wait_option);

	return ret;
	
}

/*
 *    mlock release
 *	num	Lock number 0 - 31
 */
RAW_U16 raw_munlock(RAW_MLOCK *mlock, RAW_U8 num)
{
	RAW_U16 ret;

	ret = raw_event_set(&mlock->event_ptr, 1 << num, RAW_OR);
	return ret;
}

/*
 * Create multi-lock
 */
RAW_U16 raw_mlock_create(RAW_MLOCK *mlock, RAW_U8 *name_ptr)
{
	RAW_U16 ret;

	ret = raw_event_create(&mlock->event_ptr, name_ptr, 0xffffffff);

	return ret;
}

/*
 * Delete multi-lock
 */
RAW_U16 raw_mlock_delete(RAW_MLOCK *mlock)
{
	RAW_U16 ret;

	ret = raw_event_delete(&mlock->event_ptr);

	return ret;
}

