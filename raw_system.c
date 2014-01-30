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


/*
************************************************************************************************************************
*                                    Finish interrupt
*
* Description: This function is called to when exit interrupt.
*
* Arguments  :NONE
*                
*               
*                
*                 
*				         
* Returns		
*					
* Note(s)    
*             
************************************************************************************************************************
*/
RAW_VOID raw_finish_int(void)
{

	RAW_SR_ALLOC();

	USER_CPU_INT_DISABLE();

	/*prevent raw_int_nesting 0 enter, 0 means it is processed*/
	if (raw_int_nesting == 0) {
		USER_CPU_INT_ENABLE();                                  
		return;
	}

	raw_int_nesting--;
	/*if still interrupt nested just return */
	if (raw_int_nesting) {              
		USER_CPU_INT_ENABLE();                                  
		return;
	}
	/*if system is locked then just return*/
	if (raw_sched_lock) { 

		USER_CPU_INT_ENABLE(); 
		/*if interrupt happen here, it may cause raw_int_nesting equal 0*/
		return;
	}

	/*get the highest task*/
	get_ready_task(&raw_ready_queue);

	/*if the current task is still the highest task then we do not need to have interrupt switch*/ 
	if (high_ready_obj == raw_task_active) {                 
		USER_CPU_INT_ENABLE();                                     
		return;
	}

	TRACE_INT_TASK_SWITCH(raw_task_active, high_ready_obj);
	
	/*switch to the highest task, this function is cpu related, thus need to be ported*/
	raw_int_switch();  

	USER_CPU_INT_ENABLE();  

}



/*
************************************************************************************************************************
*                                       Timer tick function
*
* Description: This function is called by timer interrupt.
*
* Arguments  :None
*                
*                 
*
*				         
* Returns		 None
*						
* Note(s)    Called by your own timer interrupt.
*
*             
************************************************************************************************************************
*/
RAW_VOID raw_time_tick(void)
{

	#if (CONFIG_RAW_TASK_0 > 0)
	
	if (raw_int_nesting) {

		RAW_ASSERT(0);
			
	}

	#endif
	
	#if (CONFIG_RAW_USER_HOOK > 0)
	raw_tick_hook();
	#endif

	/*update system time to calculate whether task timeout happens*/
	#if (CONFIG_RAW_TICK_TASK > 0)
	raw_task_semaphore_put(&tick_task_obj);
	#else
	tick_list_update();
	#endif

	/*update task time slice if possible*/
	#if (CONFIG_SCHED_FIFO_RR > 0)
	calculate_time_slice(raw_task_active->priority);
	#endif

	/*inform the timer task to update software timer*/	
	#if (CONFIG_RAW_TIMER > 0)
	call_timer_task();
	#endif
}



/*
************************************************************************************************************************
*                                    Enter interrupt
*
* Description: This function is called to when enter into interrupt.
*
* Arguments  :NONE
*                
*               
*                
*                 
*				         
* Returns		
*					
* Note(s) raw_int_nesting can directly be accessed by interrupt function with cpu interrupt disabled!
*you must invoke raw_enter_interrupt before calling interrupt function.
*you must  invoke  raw_finish_int after calling interrupt function.
* you must invoke raw_enter_interrupt and raw_finish_int in pair.
*             
************************************************************************************************************************
*/
RAW_U16 raw_enter_interrupt(void)
{
	RAW_SR_ALLOC();

	if (raw_int_nesting >= INT_NESTED_LEVEL) {  
		
		return RAW_EXCEED_INT_NESTED_LEVEL;                                                                                      
	}

	RAW_CPU_DISABLE();
	raw_int_nesting++; 
	RAW_CPU_ENABLE();

	return RAW_SUCCESS;
}




/*
************************************************************************************************************************
*                                     Get system time
*
* Description: This function is called to get system time.
*
* Arguments  :NONE
*                
*               
*                
*                 
*				         
* Returns		
*					raw_tick_count: The raw_os time.
* Note(s)    Becareful raw_tick_count will reset to 0 when it reach 0xffffffff.
*             
************************************************************************************************************************
*/

RAW_TICK_TYPE  raw_system_time_get(void)
{

	return raw_tick_count;

}


RAW_U16 block_state_post_process(RAW_TASK_OBJ  *task_ptr, RAW_VOID  **msg)
{
	RAW_U8 state;
	RAW_U16 error_status = 0;
	
	
	state = task_ptr->block_status;
	
	switch (state) {
		
		case RAW_B_OK:                             /* We got the message */
			/*if deal with msg*/	
			if (msg) {
				*msg = task_ptr->msg;
			}
			
			error_status = RAW_SUCCESS;
			break;

		case RAW_B_ABORT:                          /* Indicate that we aborted  */

			error_status = RAW_BLOCK_ABORT;
			break;

		case RAW_B_TIMEOUT:                        /* Indicate that it is timeout */

			error_status = RAW_BLOCK_TIMEOUT;
			break;

		case RAW_B_DEL:                            /* Indicate that object pended on has been deleted */

			error_status = RAW_BLOCK_DEL;
			break;

		default:
			
			RAW_ASSERT(0);
			
	}

	return error_status;
	
}



/*
************************************************************************************************************************
*                                    Set system time
*
* Description: This function is called to set system time.
*
* Arguments  :NONE
*                
*               
*                
*                 
*				         
* Returns		
*					raw_tick_count: The raw_os time.
* Note(s)    
*             
************************************************************************************************************************
*/
RAW_U16 raw_system_time_set(RAW_TICK_TYPE time)
{
	RAW_SR_ALLOC();

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;
	}
	
	RAW_CRITICAL_ENTER();
	raw_tick_count = time;
	RAW_CRITICAL_EXIT();

	return RAW_SUCCESS;

}


#if (CONFIG_SYSTEM_MEMOPT > 0)

RAW_VOID *raw_memset(RAW_VOID *src, RAW_U8 byte, RAW_U32 count)
{
	RAW_U8 *xs = src;

	while (count--) {
		
		*xs++ = byte;
	}
	
	return src;
}


RAW_VOID *raw_memcpy(RAW_VOID *dest, const RAW_VOID *src, RAW_U32 count)
{
	RAW_U8 *tmp = dest;
	const RAW_U8 *s = src;

	while (count--) {
		
		*tmp++ = *s++;
	}
	
	return dest;
}

#endif

RAW_S32 bit_search_first_one(RAW_U32 *base, RAW_U8 offset, RAW_S32 width)
{
	register RAW_U32 *cp, v;
	register RAW_S32 position;

	cp = base;
	/*caculate word position to bitmap*/
	cp += offset >> 5;
	
	/* clear all bit before offset(not include offset bit), do not need do this if offset is 32, 64, 96, etc......*/
	if (offset & 31) {
		
		#if (CONFIG_RAW_LITTLE_ENDIAN > 0)
		v = *cp & ~(((RAW_U32)1 << (offset & 31)) - 1);
		#else
		v = *cp & (((RAW_U32)1 << (32 - (offset & 31))) - 1);
		#endif
	} 

	else {
		
		v = *cp;
	}

	position = 0;
	while (position < width) {
		if (v){

			 /*Set right position first time*/
			if (!position) { 
				
				position -= (offset & 31);
			}
					
			#if  (CONFIG_RAW_LITTLE_ENDIAN > 0)

			if (!(v & 0xffff)) {
				v >>= 16;
				position += 16;
			}
			if (!(v & 0xff)) {
				v >>= 8;
				position += 8;
			}
			if (!(v & 0xf)) {
				v >>= 4;
				position += 4;
			}
			if (!(v & 0x3)) {
				v >>= 2;
				position += 2;
			}
			if (!(v & 0x1)) {
				++position;
			}

			#else

			if (!(v & 0xffff0000)) {
				v <<= 16;
				position += 16;
			}

			if (!(v & 0xff000000)) {
				v <<= 8;
				position += 8;
			}

			if (!(v & 0xf0000000)) {
				v <<= 4;
				position += 4;
			}

			if (!(v & 0xc0000000)) {
				v <<= 2;
				position += 2;
			}

			if (!(v & 0x80000000)) {
				++position;
			}
					
			#endif
		
			if (position < width) {
						
				return position;
							
			} else {
				
				return -1;
						
			}
		} 

		else {  
			
			/*Skip one world*/
			if (position) {
				
				position += 32;
			} else {
			
				position = 32 - (offset & 31);
			}
		
			v = *++cp;
		}
	}

    return -1;
}

