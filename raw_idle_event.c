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

/* 	2012-12  Created by jorya_txj
  *	xxxxxx   please added here
  */


#include <raw_api.h>

#if (CONFIG_RAW_IDLE_EVENT > 0)

#include <port_idle_config.h>


void idle_event_init(void) 
{

	ACTIVE_EVENT_STRUCT *temp;
	RAW_U8 i;

	/*Init idle task queue information, MAX_IDLE_EVENT_TASK max 64 is allowed*/
	for (i = 0; i <= MAX_IDLE_EVENT_TASK - 1; i++) {
		temp = active_idle_task[i].act;

		RAW_ASSERT(temp != 0); 
		
		temp->prio = i;
		temp->head    = 0;
		temp->tail    = 0;
		temp->nUsed   = 0;

		temp->priority_x = i & 0x7;
		temp->priority_y = i >> 3;
		
		temp->priority_bit_y = (RAW_U8 )(1 << temp->priority_y);
		temp->priority_bit_x = (RAW_U8 )(1 << temp->priority_x);

	}

	/*Init the list*/
	list_init(&raw_idle_tick_head);

}


RAW_U16 event_post(ACTIVE_EVENT_STRUCT *me, RAW_U16 sig, void *para, RAW_U8 opt_send_method)
{

	ACTIVE_EVENT_STRUCT_CB *acb;
	
 	RAW_SR_ALLOC();
	
	acb = &active_idle_task[me->prio];
	
	RAW_CRITICAL_ENTER();

	if (me->nUsed == acb->end) {

		RAW_CRITICAL_EXIT();
		return RAW_IDLE_EVENT_EXHAUSTED;
	}
	

	if (opt_send_method == SEND_TO_END) {
		
		acb->queue[me->head].sig = sig;

		acb->queue[me->head].para = para;

		me->head++;

		if (me->head == acb->end) {   
			
			me->head = 0;
			
		}   
	}

	else {

		if (me->tail == 0) { 			   
			me->tail = acb->end;
		}

		me->tail--;

		acb->queue[me->tail].sig = sig;

		acb->queue[me->tail].para = para;

	}
	
	++me->nUsed;
	
	if (me->nUsed == 1) {             
		
		raw_idle_rdy_grp |= acb->act->priority_bit_y;
		raw_rdy_tbl[acb->act->priority_y] |= acb->act->priority_bit_x;

	}

	RAW_CRITICAL_EXIT();

	return RAW_SUCCESS;

   	
}


/*
************************************************************************************************************************
*                                      Post an event (FIFO) to idle task
*
* Description: This function is called to post an event to idle task, it might be called in interrupt.
*
* Arguments  :me is the address of ACTIVE_EVENT_STRUCT
*                    -----
*                    sig is the signal which want to be posted to idle task
*		         -----
*                    para is the parameter which want to be posted to idle task. 
*
* Returns	RAW_SUCCESS: raw os return success		
*                   RAW_IDLE_EVENT_EXHAUSTED: No more msg to me.
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 idle_event_end_post(ACTIVE_EVENT_STRUCT *me, RAW_U16 sig, void *para)
{

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting && raw_sched_lock) {
		
		return int_msg_post(RAW_TYPE_IDLE_END_EVENT_POST, me, para, sig, 0, 0);
	}
	
	#endif
	
	return event_post(me, sig, para, SEND_TO_END);

}


/*
************************************************************************************************************************
*                                      Post an event (LIFO) to idle task
*
* Description: This function is called to post an event to idle task, it might be called in interrupt.
*
* Arguments  :me is the address of ACTIVE_EVENT_STRUCT
*                    -----
*                    sig is the signal which want to be posted to idle task
*		         -----
*                    para is the parameter which want to be posted to idle task. 
*
* Returns	RAW_SUCCESS: raw os return success		
*                   RAW_IDLE_EVENT_EXHAUSTED: No more msg to me.
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 idle_event_front_post(ACTIVE_EVENT_STRUCT *me, RAW_U16 sig, void *para)
{

	#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

	if (raw_int_nesting && raw_sched_lock) {
		
		return int_msg_post(RAW_TYPE_IDLE_FRONT_EVENT_POST, me, para, sig, 0, 0);
	}
	
	#endif
	
	return event_post(me, sig, para, SEND_TO_FRONT);

}



void idle_tick_isr(void) 
{

	ACTIVE_EVENT_STRUCT *a;
	
	LIST *head;
	LIST *iter;
	LIST *iter_temp;

	head = &raw_idle_tick_head;
	iter = head->next;

	/*if list is not empty*/
 	while (iter != head) {

		a =  raw_list_entry(iter, ACTIVE_EVENT_STRUCT, idle_tick_list);
		iter_temp =  iter->next;

		if (a->tick_ctr) {
			--a->tick_ctr;
			
			if (a->tick_ctr == 0) {
				list_delete(iter);
				idle_event_end_post(a, STM_TIMEOUT_SIG, 0);
			}
		}
		
		iter = iter_temp;
 	}

	
}


RAW_U16 idle_tick_arm(ACTIVE_EVENT_STRUCT *me, RAW_TICK_TYPE ticks)
{
	RAW_U16 tick_ret;

	RAW_SR_ALLOC();

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;

	}

	if (ticks == 0) {

		return RAW_IDLE_TICK_ADD_FAILED;
	}

	RAW_CPU_DISABLE();

	if (me->tick_ctr == 0) {
		me->tick_ctr = ticks;
		list_insert(&raw_idle_tick_head, &me->idle_tick_list);	
		tick_ret = RAW_SUCCESS;
	}

	else {
		tick_ret = RAW_IDLE_TICK_ADD_FAILED;
	}

	RAW_CPU_ENABLE();

	return tick_ret;

}

RAW_U16 idle_tick_disarm(ACTIVE_EVENT_STRUCT *me)
{
	RAW_U16 tick_ret;

	RAW_SR_ALLOC();

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;

	}

	RAW_CPU_DISABLE();

	if (me->tick_ctr) {

		list_delete(&me->idle_tick_list);
		me->tick_ctr = 0;
		tick_ret = RAW_SUCCESS;
	}

	else {

		tick_ret = RAW_IDLE_TICK_DELETE_FAILED;
	}

	RAW_CPU_ENABLE();

	return tick_ret;
	
}

void idle_run(void) 
{
    ACTIVE_EVENT_STRUCT *a;
	STATE_EVENT temp;
	
	ACTIVE_EVENT_STRUCT_CB *acb;
	RAW_U8 x;
	RAW_U8 y;
	RAW_U8 idle_high_priority;
	
	RAW_SR_ALLOC();

	while (1) { 
		
		RAW_CRITICAL_ENTER();

		/*if get events then process it*/
		if (raw_idle_rdy_grp) {

			y = raw_idle_map_table[raw_idle_rdy_grp];
			x = y >> 3;
			idle_high_priority = (y + raw_idle_map_table[raw_rdy_tbl[x]]);
           
			acb = &active_idle_task[idle_high_priority];
			a = active_idle_task[idle_high_priority].act;

			--a->nUsed;
			
			if (a->nUsed == 0) {         
		
				raw_rdy_tbl[a->priority_y] &= (RAW_U8)~a->priority_bit_x;
				
				if (raw_rdy_tbl[a->priority_y] == 0) {                      /* Clear event grp bit if this was only task pending */
					raw_idle_rdy_grp &= (RAW_U8)~a->priority_bit_y;
				}
			}
			
			temp.sig = acb->queue[a->tail].sig;

			temp.which_pool = acb->queue[a->tail].para;

			a->tail++;

			if (a->tail == acb->end) {                  
			    a->tail = 0;
			}

			RAW_CRITICAL_EXIT();

			#if (RAW_FSM_ACTIVE > 0)
			
			fsm_exceute(&a->super, &temp);                    
			
			#else
			
			hsm_exceute(&a->super, &temp);                   
			
			#endif
			
		}

		else {

			RAW_CRITICAL_EXIT();

			RAW_CPU_DISABLE();

			if (raw_idle_rdy_grp == 0) {
				idle_event_user();
			}
		
			RAW_CPU_ENABLE();
			
		}
		
    }
   
}

#endif


