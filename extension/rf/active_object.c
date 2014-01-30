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



STATE_EVENT *active_event_get(ACTIVE_OBJECT_STRUCT *me) 
{
	RAW_U16 err;
	STATE_EVENT *e;

	err = raw_queue_receive (&me->active_queue, RAW_WAIT_FOREVER, (RAW_VOID  **)&e);

	if (err != RAW_SUCCESS) {


		RAW_ASSERT(0);
	}

	return e;
	
}


/*
************************************************************************************************************************
*                                    Post an event to active object  
*
* Description: This function is called to post an event to active object (Implemented as FIFO way)
*
* Arguments  :me is the address of this active object
*                    ---------
*                    event is the address of sending event               
*				         
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
void active_event_post_end(ACTIVE_OBJECT_STRUCT *me, STATE_EVENT *event)
{
	RAW_U16 ret;

	RAW_SR_ALLOC();
	

	RAW_CPU_DISABLE();

	if (event->which_pool) {          
		event->ref_count++;          
	}

	RAW_CPU_ENABLE();

	ret = raw_queue_end_post(&me->active_queue, (void *)event);

	if (ret != RAW_SUCCESS) {

		RAW_ASSERT(0);

	}

	

	
	
}



/*
************************************************************************************************************************
*                                    Post an event to active object  
*
* Description: This function is called to post an event to active object (Implemented as LIFO way)
*
* Arguments  :me is the address of this active object
*                    ---------
*                    event is the address of sending event               
*				         
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
void active_event_post_front(ACTIVE_OBJECT_STRUCT *me, STATE_EVENT *event)
{

	RAW_U16 ret;

	RAW_SR_ALLOC();

	RAW_CPU_DISABLE();

	if (event->which_pool) {          
		event->ref_count++;          
	}

	RAW_CPU_ENABLE();

	ret = raw_queue_front_post(&me->active_queue, (void *)event);

	if (ret != RAW_SUCCESS) {

		RAW_ASSERT(0);

	}

	
	
}



static void active_task_function(void *pdata) 
{       
	STATE_EVENT *e;
	RAW_U16 ret;

	ACTIVE_OBJECT_STRUCT *object = pdata;


	while (object->user_data == 1) {

		e = active_event_get(object);

		hsm_exceute(&object->father, e);

		active_memory_collect(e);

	}


	ret = raw_task_delete(raw_task_identify());

	if (ret != RAW_SUCCESS) {

		RAW_ASSERT(0);

	}
	
}



/*
************************************************************************************************************************
*                                    Create an active object  
*
* Description: This function is called to post an event to active object
*
* Arguments  :me is the address of this active object
*                    ---------
*                    prio is the priority of this active object            
*			 ---------
*                    msg_start is the start address of active queue which is belonged to this active object
*			 ---------
*			 qLen is the active queue length
*			 ---------
*			 stkSto is the start stack address of this active object
*			 ---------
*			 stkSize is the stack size of this active object
*			 ---------
*			 event is the init event.
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
void active_object_create(ACTIVE_OBJECT_STRUCT *me, RAW_U8 prio,
                   RAW_VOID **msg_start, RAW_U32 qLen,
                   void *stkSto, RAW_U32 stkSize,
                   STATE_EVENT *event)
{
	RAW_U16 err;

	err = raw_queue_create(&me->active_queue, (RAW_U8 *)"queue", msg_start, qLen);

	if (err != RAW_SUCCESS) {

		RAW_ASSERT(0);
	}


	me->prio = prio;                                

	hsm_init(&me->father, event);

	me->user_data = 1;

	err = raw_task_create(&me->thread, (RAW_U8  *)"task5", me,
	                 me->prio, 0,   stkSto, 
	                 stkSize, active_task_function, 1); 

	if (err != RAW_SUCCESS) {

		RAW_ASSERT(0);
	}
	
}



/*
************************************************************************************************************************
*                                    Delete an event  object  
*
* Description: This function is called to delete an event to active object.
*
* Arguments  :me is the address of this active object
*                   
*                              
*				         
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
void active_object_delete(ACTIVE_OBJECT_STRUCT *me)
{
	RAW_U16 err;
	me->user_data = 0;                        
	err = raw_queue_delete(&me->active_queue);

	if (err != RAW_SUCCESS) {

		RAW_ASSERT(0);
	}
}



/*
************************************************************************************************************************
*                                   Post an event  to a defered queue
*
* Description: This function is called to post an event  to a defered queue.
*
* Arguments  :q is the address of the defered queue
*                    ---------
*                    event is the defered event           
*				         
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
void active_event_defer_post(RAW_QUEUE *q, STATE_EVENT *event)
{
	RAW_U16 ret;

	ret = raw_queue_end_post(q, (void *)event);

	if (ret != RAW_SUCCESS) {

		RAW_ASSERT(0);

	}
}


/*
************************************************************************************************************************
*                                   Post an event  to a defered queue
*
* Description: This function is called to post an event  to a defered queue.
*
* Arguments  :q is the address of the defered queue
*                    ---------
*                   me is the active object  to post       
*				         
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 active_event_recall(ACTIVE_OBJECT_STRUCT *me, RAW_QUEUE *q) 
{
	STATE_EVENT *event;
	RAW_U16 recalled;
	RAW_U16 err;
	RAW_SR_ALLOC();

	err = raw_queue_receive (q, RAW_NO_WAIT, (RAW_VOID  **)&event);

	if (err == RAW_SUCCESS) {
		
		RAW_CPU_DISABLE();

		if (event->which_pool) {  
			
			event->ref_count++;
		}

		RAW_CPU_ENABLE();

		active_event_post_front(me, event);

		recalled = 1;
	}

	else {

		recalled = 0;
	}


	return recalled;
}

