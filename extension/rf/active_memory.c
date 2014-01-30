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


/* 	2012-9  Created by jorya_txj
  *	xxxxxx   please added here
  */


#include <raw_api.h>
#include <rf/rf_config.h>
#include <rf/active_object.h>
#include <rf/active_time_event.h>
#include <rf/active_memory.h>
#include <rf/active_queue_broadcast.h>


/*
************************************************************************************************************************
*                                    Allocate an event from pool
*
* Description: This function is called to allocate an event block from pool
*
* Arguments  :pool is the address of memory block pool 
*                    ---------
*                    sig is the event sig              
*				         
* Returns			
*						
* Note(s) 
*
*             
************************************************************************************************************************
*/
void *active_event_memory_allocate(MEM_POOL *pool, RAW_U16 sig)
{

	STATE_EVENT *event;
	RAW_U16 ret;

	ret = raw_block_allocate(pool, (RAW_VOID **)&event);

	if (ret != RAW_SUCCESS) {

		RAW_ASSERT(0);

	}

	event->sig = sig;
	event->which_pool = pool;
	event->ref_count = 0;

	return (void *)event;
	
}



/*
************************************************************************************************************************
*                                    Collect an event from pool
*
* Description: This function is Collect an event from pool
*
* Arguments  :event is the event whichto be collected.
*                    ---------
*                             
*				         
* Returns			
*						
* Note(s)    This is the internal function and should not be called by users.	
*
*             
************************************************************************************************************************
*/
void active_memory_collect(STATE_EVENT *event) 
{
	RAW_U16 ret;

	RAW_SR_ALLOC();

	if (event->which_pool) {       

		RAW_CPU_DISABLE();

		if (event->ref_count > 1) {  
			event->ref_count--;

			RAW_CPU_ENABLE();
		}

		else {    

			RAW_CPU_ENABLE();
	
			RAW_ASSERT(event->ref_count == 1);
			ret = raw_block_release(event->which_pool, event);

			if (ret != RAW_SUCCESS) {

				RAW_ASSERT(0);
			}

		}


	}
	
}


