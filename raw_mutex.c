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

#include <raw_api.h>

#if (CONFIG_RAW_MUTEX > 0)
/*
************************************************************************************************************************
*                                      Create a mutex
*
* Description: This function is called to create a mutex.
*
* Arguments  :mutex_ptr is the address of the mutex object want to be released
*                   name_ptr is the name of this mutex
*                   priority_inherit is to specify whether  this mutex support priority inheritance
*                	policy can be RAW_MUTEX_INHERIT_POLICY or RAW_MUTEX_CEILING_POLICY
*			ceiling_prio is the highest priority  for mutex when RAW_MUTEX_CEILING_POLICY is specified.
*			
* Returns	RAW_SUCCESS: raw os return success
* Note(s)    	if policy is RAW_MUTEX_INHERIT_POLICY, then ceiling_prio is useless and you can assign any value to it.
* 			RAW_BLOCKED_WAY_FIFO is not a recommanded way for mutex, RAW_BLOCKED_WAY_PRIO id prefered
*                   if CONFIG_RAW_ZERO_INTERRUPT is set, then ceiling_prio 0 is not allowed!
*             
************************************************************************************************************************
*/
RAW_U16 raw_mutex_create(RAW_MUTEX *mutex_ptr, RAW_U8 *name_ptr, RAW_U8 policy, RAW_U8 ceiling_prio)
{
	
	
	#if (RAW_MUTEX_FUNCTION_CHECK > 0)

	if (mutex_ptr == 0) {
		return RAW_NULL_OBJECT;
	}

	if ((policy != RAW_MUTEX_CEILING_POLICY) && (policy != RAW_MUTEX_INHERIT_POLICY) && (policy != RAW_MUTEX_NONE_POLICY)) {

		return RAW_MUTEX_NO_POLICY;
	}
	
	#endif
	
	/*Init the list*/
	list_init(&mutex_ptr->common_block_obj.block_list);
	mutex_ptr->common_block_obj.block_way = RAW_BLOCKED_WAY_PRIO;
	mutex_ptr->common_block_obj.name  =  name_ptr;
	/*No one occupy mutex yet*/
	mutex_ptr->mtxtsk 		= 0;
	mutex_ptr->mtxlist 		= 0;

	mutex_ptr->policy = policy;

	#if (CONFIG_RAW_TASK_0 > 0)

	if (policy == RAW_MUTEX_CEILING_POLICY) {
		
		if (ceiling_prio == 0) {
		
			return RAW_CEILING_PRIORITY_NOT_ALLOWED;
		}
	}
	
	#endif
	
	mutex_ptr->ceiling_prio = ceiling_prio;
	mutex_ptr->common_block_obj.object_type = RAW_MUTEX_OBJ_TYPE;

	TRACE_MUTEX_CREATE(raw_task_active, mutex_ptr, name_ptr, policy, ceiling_prio);
	
	return RAW_SUCCESS;

}



/*
 * Limit the priority change by mutex at task priority change
 *    1.If the 'tcb' task locks mutex, cannot set lower priority than the
 *	highest priority in all mutexes which hold lock. In such case,
 *	return the highest priority of locked mutex.
 *    2.If mutex with TA_CEILING attribute is locked or waiting to be locked,
 *	cannot set higher priority than the lowest within the highest
 *	priority limit of mutex with TA_CEILING attribute.
 *	In this case, return E_ILUSE.
 *    3.Other than above, return the 'priority'.
 */
RAW_U8 chg_pri_mutex(RAW_TASK_OBJ *tcb, RAW_U8 priority, RAW_U16 *error)
{
	RAW_MUTEX	*mtxcb;
	RAW_U8	hi_pri, low_pri, pri;
	RAW_TASK_OBJ *first_block_task;
	LIST *block_list_head;
	
	hi_pri  = priority;
	low_pri = 0u;
	
	mtxcb = (RAW_MUTEX	*)(tcb->block_obj);
	
	if (mtxcb) {

		/*if it is blocked on mutex*/
		if (mtxcb->common_block_obj.object_type == RAW_MUTEX_OBJ_TYPE) {
			
			if (mtxcb->policy == RAW_MUTEX_CEILING_POLICY) {
				pri = mtxcb->ceiling_prio;
				
				if (pri > low_pri) {
					low_pri = pri;
				}
			}
		}
	}

	/* Locked Mutex */
	pri = hi_pri;
	for (mtxcb = tcb->mtxlist; mtxcb != 0; mtxcb = mtxcb->mtxlist) {
		switch (mtxcb->policy) {
			
		  case RAW_MUTEX_CEILING_POLICY:
			pri = mtxcb->ceiling_prio;
			if ( pri > low_pri ) {
				low_pri = pri;
			}
			break;
			
		  case RAW_MUTEX_INHERIT_POLICY:
		  	
			block_list_head = &mtxcb->common_block_obj.block_list;
			
			if (!is_list_empty(block_list_head)) {
				first_block_task = raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list); 
				pri = first_block_task->priority;
			}
			
			break;
			
		  default:
			/* nothing to do */
			break;
		}

		/*can not set lower priority than the highest priority in all mutexes which hold lock*/
		if (pri < hi_pri) {
			hi_pri = pri;
		}
	}

	if (priority < low_pri) {
		
		*error = RAW_EXCEED_CEILING_PRIORITY;
		return RAW_EXCEED_CEILING_PRIORITY;
	}

	*error = RAW_SUCCESS;
	return hi_pri;
}




/*
 * Release the lock and delete it from list, and then adjust the
 * priority of task.
 * Set the highest priority between listed below:
 *	(A) The highest priority in all mutexes in which 'tcb' task locks.
 *	(B) The base priority of 'tcb' task.
 */
static RAW_VOID release_mutex(RAW_TASK_OBJ *tcb, RAW_MUTEX *relmtxcb)
{
	RAW_MUTEX	*mtxcb, **prev;
	RAW_U8	newpri, pri;
	RAW_TASK_OBJ *first_block_task;
	LIST *block_list_head;
	
	/* (B) The base priority of task */
	newpri = tcb->bpriority;

	/* (A) The highest priority in mutex which is locked */
	pri = newpri;
	prev = &tcb->mtxlist;
	while ((mtxcb = *prev) != 0) {
		if (mtxcb == relmtxcb) {
			/* Delete self from list and tcb->mtxlist point to next*/
			*prev = mtxcb->mtxlist;
			continue;
		}

		switch (mtxcb->policy) {
		  case RAW_MUTEX_CEILING_POLICY:
			pri = mtxcb->ceiling_prio;
			break;
			
		  case RAW_MUTEX_INHERIT_POLICY:
		  	
		  	block_list_head = &mtxcb->common_block_obj.block_list;
			
			if (!is_list_empty(block_list_head)) {
				first_block_task = raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list); 
				pri = first_block_task->priority;
			}
			
			break;
			
		  default:
			break;
		}
		if (newpri > pri) {
			newpri = pri;
		}

		prev = &mtxcb->mtxlist;
	}

	if ( newpri != tcb->priority ) {
		/* Change priority of lock get task */
		change_internal_task_priority(tcb, newpri);

		TRACE_MUTEX_RELEASE(raw_task_active, tcb, newpri);
	}
	
}


/*
 * Processing if the priority of wait task changes
 */
RAW_VOID mtx_chg_pri(RAW_TASK_OBJ *tcb, RAW_U8 oldpri)
{
	RAW_MUTEX		*mtxcb;
	RAW_TASK_OBJ	*mtxtsk;

	mtxcb = (RAW_MUTEX	*)(tcb->block_obj);

	/*mutex_recursion_levels can never deeper than 5 levles, anyway it is the design fault*/
	if (mutex_recursion_levels > 5) {

		return;
	}

	mutex_recursion_levels++;

	/*update max mutex recursion levels for debug, mainly help to find the mutex design fault*/
	if (mutex_recursion_levels > mutex_recursion_max_levels) {

		mutex_recursion_max_levels = mutex_recursion_levels;
	}
	
	if (mtxcb->common_block_obj.object_type == RAW_MUTEX_OBJ_TYPE) {
		
		if (mtxcb->policy == RAW_MUTEX_INHERIT_POLICY) {
			mtxtsk = mtxcb->mtxtsk;
			
			if (mtxtsk->priority > tcb->priority) {

				/* Since the highest priority of the lock wait task
		  		 became higher, raise the lock get task priority
		   		higher */
				change_internal_task_priority(mtxtsk, tcb->priority);

			}

			/*the highest priority task blocked on this mutex may decrease priority so reset the mutex task priority*/
			else if (mtxtsk->priority == oldpri) {

				release_mutex(mtxtsk, 0);
			}

			else {
				
				/*tcb->priority<= mtxtsk->priority and mtxtsk->priority != oldpri*/

			}
			
		}
		
	}

	mutex_recursion_levels--;
	
}


/*
 * Processing if the task blocked on mutex is timeout or aborted or deleted
 */
RAW_VOID mutex_state_change(RAW_TASK_OBJ *tcb)
{
	RAW_MUTEX		*mtxcb;
	RAW_TASK_OBJ	*mtxtsk;

	mtxcb = (RAW_MUTEX	*)(tcb->block_obj);
	
	if (mtxcb->common_block_obj.object_type == RAW_MUTEX_OBJ_TYPE) {
				
		if (mtxcb->policy == RAW_MUTEX_INHERIT_POLICY) {
			mtxtsk = mtxcb->mtxtsk;
			
			/*the highest priority task blocked on this mutex may decrease priority so reset the mutex task priority*/
			if(mtxtsk->priority == tcb->priority) {

				release_mutex(mtxtsk, 0);
			}
			
		}
	}

}



/*
************************************************************************************************************************
*                                     Get a mutex
*
* Description: This function is called to get a mutex.
*
* Arguments  :mutex_ptr: is the address of the mutex object want to be released
*                   wait_option: if equals 0 return immeadiately and 0xffffffff wait foreverand 
*						 and others are timeout value
*                   -----------------------------
*                   wait_option
*                    RAW_NO_WAIT (0x00000000)
*						 RAW_WAIT_FOREVER (0xFFFFFFFF)
*						 timeout value (0x00000001
*							  					through
*												0xFFFFFFFE)
*				         
* Returns		
*					RAW_SUCCESS : Get mutex success.
*               RAW_BLOCK_ABORT: mutex is aborted by other task or ISR.
*               RAW_NO_PEND_WAIT: mutex is not got and option is RAW_NO_WAIT.
*               RAW_SCHED_DISABLE: system is locked ant task is not allowed block.
*               RAW_BLOCK_DEL: if this mutex is deleted
*
* Note(s)    Any task pended on this semphore will be waked up and will return RAW_B_DEL.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_mutex_get(RAW_MUTEX *mutex_ptr, RAW_TICK_TYPE wait_option)

{
	RAW_U16 		error_status;
	RAW_TASK_OBJ	*mtxtsk;
	
	RAW_SR_ALLOC();

	#if (RAW_MUTEX_FUNCTION_CHECK > 0)


	if (mutex_ptr == 0) {
		return RAW_NULL_OBJECT;
	}

	
	if (raw_int_nesting) {
		
		return RAW_NOT_CALLED_BY_ISR;
		
	}
	
	#endif

	RAW_CRITICAL_ENTER();

	if (mutex_ptr->common_block_obj.object_type != RAW_MUTEX_OBJ_TYPE) {
		
		RAW_CRITICAL_EXIT();  
		return RAW_ERROR_OBJECT_TYPE;
	}
	
		/*if the same task get the same mutex again, it causes deadlock*/ 
	if (raw_task_active == mutex_ptr->mtxtsk) {
		
		mutex_ptr->owner_nested++;
  		RAW_CRITICAL_EXIT();   
		return RAW_MUTEX_OWNER_NESTED;
   }


	if (mutex_ptr->policy == RAW_MUTEX_CEILING_POLICY) {

		if (raw_task_active->bpriority < mutex_ptr->ceiling_prio) {
			/* Violation of highest priority limit */
			RAW_CRITICAL_EXIT();

			TRACE_MUTEX_EX_CE_PRI(raw_task_active, mutex_ptr, wait_option);
			return RAW_EXCEED_CEILING_PRIORITY;
		}

	}
	

	mtxtsk = mutex_ptr->mtxtsk;

	if (mtxtsk == 0) {
		/* Get lock */
		mutex_ptr->mtxtsk = raw_task_active;
		mutex_ptr->mtxlist = raw_task_active->mtxlist;
		raw_task_active->mtxlist = mutex_ptr;
		mutex_ptr->owner_nested = 1u;
		
		if (mutex_ptr->policy == RAW_MUTEX_CEILING_POLICY) {
			
			if (raw_task_active->priority > mutex_ptr->ceiling_prio) {
				/* Raise its own task to the highest
				   priority limit */
				change_internal_task_priority(raw_task_active, mutex_ptr->ceiling_prio);
			}

		}

		RAW_CRITICAL_EXIT();

		TRACE_MUTEX_GET(raw_task_active, mutex_ptr, wait_option);
		
		return RAW_SUCCESS;
	}
	

	/*Cann't get mutex, and return immediately if wait_option is  RAW_NO_WAIT*/
	if (wait_option == RAW_NO_WAIT) { 

		RAW_CRITICAL_EXIT();

		return RAW_NO_PEND_WAIT;

	}

	/*system is locked so task can not be blocked just return immediately*/
	SYSTEM_LOCK_PROCESS();

    /*if current task is a higher priority task and block on  the mutex
	  *priority inverse condition happened, priority inherit method is used here*/

	if (mutex_ptr->policy == RAW_MUTEX_INHERIT_POLICY) {
		
		if (raw_task_active->priority < mtxtsk->priority) {

			TRACE_TASK_PRI_INV(raw_task_active, mtxtsk);
			change_internal_task_priority(mtxtsk, raw_task_active->priority);

		}
	}

	
	/*Any way block the current task*/
	raw_pend_object((RAW_COMMON_BLOCK_OBJECT  *)mutex_ptr, raw_task_active, wait_option);

	RAW_CRITICAL_EXIT();

	TRACE_MUTEX_GET_BLOCK(raw_task_active, mutex_ptr, wait_option);

	/*find the next highest priority task ready to run*/
	raw_sched();                                             

	/*So the task is waked up, need know which reason cause wake up.*/
	error_status = block_state_post_process(raw_task_active, 0);
	
	return error_status;
}

/*
************************************************************************************************************************
*                                       Release a mutex
*
* Description: This function is called to release a mutex.
*
* Arguments  :mutex_ptr is the address of the mutex object want to be released
*                
*                
*				         
* Returns		RAW_SUCCESS: raw os return success
* Note(s)    Any task pended on this semphore will be waked up and will return RAW_B_DEL.
*
*             
************************************************************************************************************************
*/
RAW_U16 raw_mutex_put(RAW_MUTEX *mutex_ptr)
{

	LIST 				*block_list_head;
	RAW_TASK_OBJ   		*tcb;
	
	RAW_SR_ALLOC();

	#if (RAW_MUTEX_FUNCTION_CHECK > 0)

	if (mutex_ptr == 0) {
		return RAW_NULL_OBJECT;
	}

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}
	
	#endif

	RAW_CRITICAL_ENTER();

	if (mutex_ptr->common_block_obj.object_type != RAW_MUTEX_OBJ_TYPE) {
		
		RAW_CRITICAL_EXIT();  
		return RAW_ERROR_OBJECT_TYPE;
	}


	/*Must release the mutex by self*/
	if (raw_task_active != mutex_ptr->mtxtsk) {           
		RAW_CRITICAL_EXIT();
		return RAW_MUTEX_NOT_RELEASE_BY_OCCYPY;
	}

	mutex_ptr->owner_nested--;

	if (mutex_ptr->owner_nested) {

		RAW_CRITICAL_EXIT();
		return RAW_MUTEX_OWNER_NESTED;

	}

	release_mutex(raw_task_active, mutex_ptr);


	block_list_head = &mutex_ptr->common_block_obj.block_list;

	/*if no block task on this list just return*/
	if (is_list_empty(block_list_head)) {        
		/* No wait task */
		mutex_ptr->mtxtsk = 0;                                    
		RAW_CRITICAL_EXIT();

		TRACE_MUTEX_RELEASE_SUCCESS(raw_task_active, mutex_ptr);
		
		return RAW_SUCCESS;
	}

	
	/* there must have task blocked on this mutex object*/ 																												
	tcb = raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list);

	/*Wake up the occupy task, which is the highst priority task on the list*/																										 
	raw_wake_object(tcb);

	/* Change mutex get task */
	mutex_ptr->mtxtsk = tcb;
	mutex_ptr->mtxlist = tcb->mtxlist;
	tcb->mtxlist = mutex_ptr;
	mutex_ptr->owner_nested = 1u;
	
	if (mutex_ptr->policy == RAW_MUTEX_CEILING_POLICY) {
		
		if (tcb->priority > mutex_ptr->ceiling_prio) {
		/* Raise the priority of the task that
		got lock to the highest priority limit */
			change_internal_task_priority(tcb, mutex_ptr->ceiling_prio);
			
		}
		
	}

	TRACE_MUTEX_WAKE_TASK(raw_task_active, tcb);
	
	RAW_CRITICAL_EXIT();

	raw_sched();                                       

	return RAW_SUCCESS;
	
}


RAW_VOID raw_task_free_mutex(RAW_TASK_OBJ *tcb)
{
	RAW_MUTEX	*mtxcb, *next_mtxcb;
	RAW_TASK_OBJ	*next_tcb;
	LIST 				*block_list_head;
	
	next_mtxcb = tcb->mtxlist;
	while ((mtxcb = next_mtxcb) != 0) {
		next_mtxcb = mtxcb->mtxlist;

		block_list_head = &mtxcb->common_block_obj.block_list;
		
		if (!is_list_empty(block_list_head)) {
			
			next_tcb = raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list);

			/* Wake wait task */
			raw_wake_object(next_tcb);

			/* Change mutex get task */
			mtxcb->mtxtsk = next_tcb;
			mtxcb->mtxlist = next_tcb->mtxlist;
			next_tcb->mtxlist = mtxcb;

			if (mtxcb->policy == RAW_MUTEX_CEILING_POLICY) {
				if (next_tcb->priority > mtxcb->ceiling_prio) {
					/* Raise the priority for the task
					   that got lock to the highest
					   priority limit */
					change_internal_task_priority(next_tcb, mtxcb->ceiling_prio);
				}
			}
		} 

		else {
			/* No wait task */
			mtxcb->mtxtsk = 0;
		}
		
	}
	
}



/*
************************************************************************************************************************
*                                       Delete a mutex
*
* Description: This function is called to delete a mutex.
*
* Arguments  :mutex_ptr is the address of mutex object want to be deleted.
*                
*                
*				         
* Returns		RAW_SUCCESS: raw os return success.
* Note(s)    Any task pended on this mutex will be waked up and will return RAW_BLOCK_DEL.
*					RAW_MUTEX object is reset to zro.
*
*             
************************************************************************************************************************
*/
#if (CONFIG_RAW_MUTEX_DELETE > 0)

RAW_U16 raw_mutex_delete(RAW_MUTEX *mutex_ptr)
{
	LIST *block_list_head;
	
	RAW_SR_ALLOC();

	#if (RAW_MUTEX_FUNCTION_CHECK > 0)

	if (mutex_ptr == 0) {
		return RAW_NULL_OBJECT;
	}
	
	#endif
	
	RAW_CRITICAL_ENTER();

	if (mutex_ptr->common_block_obj.object_type != RAW_MUTEX_OBJ_TYPE) {
		
		RAW_CRITICAL_EXIT();  
		return RAW_ERROR_OBJECT_TYPE;
	}

	block_list_head = &mutex_ptr->common_block_obj.block_list;
	
	mutex_ptr->common_block_obj.object_type = 0u;

	if (mutex_ptr->mtxtsk) {
		release_mutex(mutex_ptr->mtxtsk, mutex_ptr);
	}
	
	/*All task blocked on this mutex is waken up*/
	while (!is_list_empty(block_list_head)) {
		delete_pend_obj(raw_list_entry(block_list_head->next, RAW_TASK_OBJ, task_list));	
	}              

	RAW_CRITICAL_EXIT();

	TRACE_MUTEX_DELETE(raw_task_active, mutex_ptr);
		
	raw_sched(); 
	
	return RAW_SUCCESS;
}
#endif

#endif

