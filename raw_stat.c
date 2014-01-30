/*
     raw os - Copyright (C)  Lingjun Chen(jorya_txj).

    This file is part of raw os.

    raw os is free software; you can redistribute it it under the terms of the 
    GNU General Public License as published by the Free Software Foundation; 
    either version 3 of the License, or  (at your option) any later version.

    raw os is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; 
    without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A 
PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. if not, write email to jorya.txj@gmail.com
                                      ---

    A special exception to the LGPL can be applied should you wish to 
distribute
    a combined work that includes raw os, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/


/* 	2014-1  Created by jorya_txj
  *	xxxxxx   please added here
  */

#include <raw_api.h>


#if (RAW_CPU_STACK_DOWN > 0)

void raw_stack_check(void)
{
	PORT_STACK *task_stack_start;

	task_stack_start = raw_task_active->task_stack_base;

	/*statck check method 1*/
	if (*task_stack_start) {

		RAW_ASSERT(0);
	}

	/*statck check method 2*/
	if ((PORT_STACK *)(raw_task_active->task_stack) < task_stack_start) {

		RAW_ASSERT(0);
	}

}


#else 

void raw_stack_check(void)
{
	PORT_STACK *task_stack_start;
	PORT_STACK *task_stack_end;

	task_stack_start = raw_task_active->task_stack_base;
	
	task_stack_end = task_stack_start + raw_task_active->stack_size;

	if (*(task_stack_end - 1)) {

		RAW_ASSERT(0);
	}

	if ((PORT_STACK *)(raw_task_active->task_stack) > task_stack_end) {

		RAW_ASSERT(0);
	}

}

#endif

#if (RAW_SCHE_LOCK_MEASURE_CHECK > 0)

void sche_disable_measure_start(void)
{
	/*start measure system lock time*/
	if (raw_sched_lock == 0u) {
		raw_sche_disable_time_start = RAW_CPU_TIME_GET();
	}


}


void sche_disable_measure_stop(void)
{
	PORT_TIMER_TYPE diff;

	/*stop measure system lock time*/
	if (raw_sched_lock == 1u) {
		
		diff = RAW_CPU_TIME_GET() - raw_sche_disable_time_start;

		if (raw_sche_disable_time_max < diff) {

			raw_sche_disable_time_max = diff;
		}
	}
}


#endif

#if (RAW_CPU_INT_DIS_MEASURE_CHECK > 0)


void int_disable_measure_start(void)
{

	int_disable_times++;

	/*start measure interrupt disable time*/	
	if (int_disable_times == 1u) {

		raw_int_disable_time_start = RAW_CPU_TIME_GET();
	}


}



void int_disable_measure_stop(void)
{
	PORT_TIMER_TYPE diff;
	
	int_disable_times--;

	/*stop measure interrupt enable time*/
	if (int_disable_times == 0u) {

		diff = RAW_CPU_TIME_GET() - raw_int_disable_time_start;

		if (raw_int_disable_time_max < diff) {

			raw_int_disable_time_max = diff;
		}

	}


}



#endif

#if (RAW_CONFIG_CPU_TIME > 0)

void measure_overhead(void)
{
	PORT_TIMER_TYPE diff;

	PORT_TIMER_TYPE m1;
	PORT_TIMER_TYPE m2;


	m1 = RAW_CPU_TIME_GET();

	RAW_CPU_TIME_GET();
	RAW_CPU_TIME_GET();

	m2 = RAW_CPU_TIME_GET();

	diff = m2 - m1;

	/*measure time overhead*/
	system_meaure_overhead = diff;

}


#endif

