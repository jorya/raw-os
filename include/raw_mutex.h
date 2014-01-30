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
  

#ifndef RAW_MUTEX_H
#define RAW_MUTEX_H

#define RAW_MUTEX_INHERIT_POLICY	0x00000002U	/* Priority inherited protocol */
#define RAW_MUTEX_CEILING_POLICY	0x00000003U	/* Upper limit priority protocol */
#define RAW_MUTEX_NONE_POLICY		0x00000004U /*None policy is used*/

typedef struct RAW_MUTEX
{ 
	RAW_COMMON_BLOCK_OBJECT       common_block_obj;
	
	/*mutex owner task*/
	RAW_TASK_OBJ                  *mtxtsk;

	struct RAW_MUTEX              *mtxlist;	/* Mutex get list */

	RAW_U8                        policy;
	RAW_U8                        ceiling_prio;
	RAW_U8                        owner_nested;
	
} RAW_MUTEX;

RAW_U16 raw_mutex_create(RAW_MUTEX *mutex_ptr, RAW_U8 *name_ptr, RAW_U8 policy, RAW_U8 ceiling_prio);

RAW_U16 raw_mutex_get(RAW_MUTEX *mutex_ptr, RAW_TICK_TYPE wait_option);
RAW_U16 raw_mutex_put(RAW_MUTEX *mutex_ptr);

#if (CONFIG_RAW_MUTEX_DELETE > 0)
RAW_U16  raw_mutex_delete(RAW_MUTEX *mutex_ptr);
#endif


#endif


