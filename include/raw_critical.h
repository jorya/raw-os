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


/* 	2012-11  Created by jorya_txj
  *	xxxxxx   please added here
  */


#ifndef RAW_CRITICAL_H
#define RAW_CRITICAL_H


#if (RAW_CPU_INT_DIS_MEASURE_CHECK > 0)
                                                                      
                                                                       
#define  RAW_CPU_DISABLE()                                       \
		do {                                                     \
			USER_CPU_INT_DISABLE();                              \
			int_disable_measure_start();                         \
		} while (0)

#define  RAW_CPU_ENABLE()                                       \
		do {                                                    \
			int_disable_measure_stop();                         \
			USER_CPU_INT_ENABLE();                              \
		} while (0)


#else

#define  RAW_CPU_DISABLE()                                      \
		do {                                                    \
			USER_CPU_INT_DISABLE();                             \
		} while (0)

#define  RAW_CPU_ENABLE()                                       \
		do {                                                    \
			USER_CPU_INT_ENABLE();                              \
		} while (0)
		
#endif


#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

                                                              
#define  RAW_CRITICAL_ENTER()                                    \
		do {                                                     \
			USER_CPU_INT_DISABLE();                                   \
			raw_sched_lock++;                                    \
			USER_CPU_INT_ENABLE();                                    \
		} while (0)


	 
#define  RAW_CRITICAL_EXIT()                                     \
		do {                                                     \
			if (raw_sched_lock == 1u) {                           \
				hybrid_int_process();                            \
			}                                                    \
			else {                                               \
				USER_CPU_INT_DISABLE();                               \
				raw_sched_lock--;                                \
				USER_CPU_INT_ENABLE();                                \
			}                                                    \
		} while (0)


    
#define SYSTEM_LOCK_PROCESS()	\
		do {\
			if (raw_sched_lock >= 2u) {\
				RAW_CRITICAL_EXIT();\
				return RAW_SCHED_DISABLE;\
			}\
		} while (0)


#define SYSTEM_LOCK_PROCESS_QUEUE()	\
					do {\
						if (raw_sched_lock >= 2u) {\
							*msg = (RAW_VOID *)0;\
							RAW_CRITICAL_EXIT();\
							return RAW_SCHED_DISABLE;\
						}\
					} while (0)

#define SYSTEM_LOCK_PROCESS_QUEUE_SIZE()	\
					do {\
						if (raw_sched_lock >= 2u) {\
							*msg_ptr    = 0;\
							*receive_size = 0;\
							RAW_CRITICAL_EXIT();\
							return RAW_SCHED_DISABLE;\
						}\
					} while (0)


#else                                          


#define  RAW_CRITICAL_ENTER()                   RAW_CPU_DISABLE()          
#define  RAW_CRITICAL_EXIT()                    RAW_CPU_ENABLE() 			  

 
#define SYSTEM_LOCK_PROCESS()	\
					do {\
						if (raw_sched_lock) {\
							RAW_CRITICAL_EXIT();\
							return RAW_SCHED_DISABLE;\
						}\
					} while (0)


#define SYSTEM_LOCK_PROCESS_QUEUE()	\
					do {\
						if (raw_sched_lock) {\
							*msg = (RAW_VOID *)0;\
							RAW_CRITICAL_EXIT();\
							return RAW_SCHED_DISABLE;\
						}\
					} while (0)

#define SYSTEM_LOCK_PROCESS_QUEUE_SIZE()	\
					do {\
						if (raw_sched_lock) {\
							*msg_ptr	= 0;\
							*receive_size = 0;\
							RAW_CRITICAL_EXIT();\
							return RAW_SCHED_DISABLE;\
						}\
					} while (0)


#endif

#endif

