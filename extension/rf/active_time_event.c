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



#include 	<raw_api.h>
#include 	<rf/rf_config.h>
#include 	<rf/active_object.h>
#include 	<rf/active_time_event.h>
#include 	<rf/active_memory.h>
#include 	<rf/active_queue_broadcast.h>



static void active_time_event_post(void *para)
{
	TIME_EVENT_STRUCT *me = para;

	active_event_post_end(me->active_object, &me->father);
	
}




/*
************************************************************************************************************************
*                                   Init the time event for post specified sig
*
* Description: This function is to init the time event for post specified sig
*
* Arguments  :me is the TIME_EVENT_STRUCT object
*                   --------------
*                   sig is the timer want to fire
* Returns			
*						
* Note(s)   
*
*             
************************************************************************************************************************
*/
void active_time_event_create(TIME_EVENT_STRUCT *me, RAW_U16 sig) 
{
	RAW_U16 ret;

	if (sig < STM_USER_SIG) {

		RAW_ASSERT(0);
	}


	me->active_object= 0;
	me->father.sig = sig;
	me->father.which_pool = 0;
	
	ret = raw_timer_create(&me->timer, (RAW_U8  *)"timer", active_time_event_post, 0,1,0,0);

	if (ret != RAW_SUCCESS) {

		RAW_ASSERT(0);

	}
	
}


/*
************************************************************************************************************************
*                                  Fire to active_object with the specified sig
*
* Description: This function is to fire to active_object with the specified sig
*
* Arguments  :me is the TIME_EVENT_STRUCT object
*                   --------------
*                   active_object is the timer want fire to
*                    --------------
*                    ticks is how long need to fire.
*                    -----------------------
*                    once is just fire once if once is set to 1.
* Returns			
*						
* Note(s)   If fire a once timer, just reuse this function every time, but you have to deactivate the timer if timer is not once before use it.
*
*             
************************************************************************************************************************
*/
void active_time_event_fire(TIME_EVENT_STRUCT *me, ACTIVE_OBJECT_STRUCT *active_object, RAW_TICK_TYPE ticks, RAW_U8 once)
{
	RAW_U16 ret;

	me->active_object = active_object;


	if (once) {

		ret = raw_timer_change(&me->timer, ticks, 0);
	}

	else {

		ret = raw_timer_change(&me->timer, ticks, ticks);

	}

	if (ret != RAW_SUCCESS) {

		RAW_ASSERT(0);
	}

	ret = raw_timer_activate(&me->timer, me);

	if (ret != RAW_SUCCESS) {

		RAW_ASSERT(0);
	}
	
}


/*
************************************************************************************************************************
*                                   Deactivate the timer
*
* Description: This function is to deactivate the timer
*
* Arguments  :me is the TIME_EVENT_STRUCT object
*                   --------------
*                 
* Returns			
*						
* Note(s)   
*
*             
************************************************************************************************************************
*/
RAW_U16 active_time_event_deactivate(TIME_EVENT_STRUCT *me)
{
	RAW_U16 ret;
	ret = raw_timer_deactivate(&me->timer);

	return ret;
}


