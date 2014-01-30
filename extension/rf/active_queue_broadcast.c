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


LIST	active_queue_head[ACTIVE_MAX_BROADCAST_SIGNAL];



/*
************************************************************************************************************************
*                                   Init the broadcast queue list
*
* Description: This function is to init the broadcast queue list
*
* Arguments  :
*                   			         
* Returns			
*						
* Note(s)    This function must be called before using active_broadcast_queue_register.
*
*             
************************************************************************************************************************
*/
void active_broadcast_queue_init()
{

	RAW_U16 i;

	for (i = 0; i < ACTIVE_MAX_BROADCAST_SIGNAL; i++ ) {
		list_init(&active_queue_head[i]);
	}

}


/*
************************************************************************************************************************
*                                   Register the sig for the specied active object
*
* Description: This function is to register the sig for the specied active object
*
* Arguments  :me is the active object which is to be registered.
*                     ---------------------
*                    sig is the signal which is to be registered with
* Returns			
*						
* Note(s)    This function must be called before using active_queue_event_broadcast.
*
*             
************************************************************************************************************************
*/
void active_broadcast_queue_register(ACTIVE_OBJECT_STRUCT  *me, RAW_U16 sig)
{

	RAW_SR_ALLOC();

	sig -= STM_USER_SIG;
	

	if (sig >= ACTIVE_MAX_BROADCAST_SIGNAL) {

		RAW_ASSERT(0);

	}

	RAW_CPU_DISABLE();

	list_insert(&active_queue_head[sig], &me->active_queue_list[sig]);

	RAW_CPU_ENABLE();

	
}


/*
************************************************************************************************************************
*                                  Broadcast  event
*
* Description: This function is called to  broadcast  event sig , if active object was register with this event sig, it will get this event.
*
* Arguments  :e is the event to broadcast
*                   			         
* Returns			
*						
* Note(s)   
*
*             
************************************************************************************************************************
*/
void active_queue_event_broadcast(STATE_EVENT *e)
{
	LIST    									 *iter;
	LIST    									 *iter_temp;
	LIST    									 *event_head_ptr;
	ACTIVE_OBJECT_STRUCT                         *me;
	RAW_U16 sig;
	
	RAW_SR_ALLOC();

	sig = e->sig - STM_USER_SIG;
	
	event_head_ptr = &active_queue_head[sig];

	iter = event_head_ptr->next;


	RAW_CPU_DISABLE();

	if (e->which_pool) {

		e->ref_count++;          
	}

	RAW_CPU_ENABLE();

	while (iter != event_head_ptr) {

		iter_temp = iter->next;

		me =  list_entry(iter, ACTIVE_OBJECT_STRUCT, active_queue_list[sig]);

		active_event_post_end(me, e);
		iter = iter_temp;

	}

	active_memory_collect(e);

}



/*
************************************************************************************************************************
*                                    Unregister the sig for the specied active object
*
*
* Description: This function is to unregister the sig for the specied active object
*
* Arguments  :me is the active object which is to be unregistered.
*                     ---------------------
*                    sig is the signal which is to be unregistered with
* Returns			
*						
* Note(s)    you need be great careful to call this function.It is suggested call this function after or before active_queue_event_broadcast!
*
*             
************************************************************************************************************************
*/
void active_broadcast_queue_unregister(ACTIVE_OBJECT_STRUCT  *me, RAW_U16 sig)
{
	RAW_SR_ALLOC();

	RAW_CPU_DISABLE();

	list_delete(&me->active_queue_list[sig]);
	
	RAW_CPU_ENABLE();

}

