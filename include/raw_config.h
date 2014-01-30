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


#ifndef RAW_CONFIG_H
#define RAW_CONFIG_H

/*enable system zero interrupt*/
#define CONFIG_RAW_ZERO_INTERRUPT                                   1

/*enable system embedded trace module*/
#define CONFIG_RAW_TRACE_ENABLE                                     1

/*enable system mpu memory protect module*/
#define CONFIG_RAW_MPU_ENABLE                                       0

/*almost cpu stack is from high to low*/
#define RAW_CPU_STACK_DOWN                                          1

/*Max system priority*/
#define CONFIG_RAW_PRIO_MAX                                         62

/*if cpu is little endian, please set it to 1.If not set it to 0*/
#define CONFIG_RAW_LITTLE_ENDIAN                                    1

/*enable SCHED_FIFO and SCHED_RR support*/
#define CONFIG_SCHED_FIFO_RR                                        1

/*enable system debug*/
#define CONFIG_RAW_DEBUG                                            1

/*enable user data pointer*/
#define	CONFIG_USER_DATA_POINTER                                    1

/*enable system default memset and memcpy, you can also use your choice*/
#define CONFIG_SYSTEM_MEMOPT                                        0

/*tick numbers per second*/
#define RAW_TICKS_PER_SECOND                                        100

/*timer frequency = RAW_TICKS_PER_SECOND /  RAW_TIMER_RATE*/
#define RAW_TIMER_RATE                                              1


/*RAW OS FUNCTION MODULE*/

#define CONFIG_RAW_BYTE                                             1
#define CONFIG_RAW_BLOCK                                            1
#define CONFIG_RAW_TIMER                                            1
#define CONFIG_RAW_SEMAPHORE                                        1
#define CONFIG_RAW_MUTEX                                            1
#define CONFIG_RAW_EVENT                                            1
#define CONFIG_RAW_QUEUE                                            1
#define CONFIG_RAW_QUEUE_BUFFER                                     1
#define CONFIG_RAW_QUEUE_SIZE                                       1
#define CONFIG_RAW_MQUEUE                                           1
#define CONFIG_RAW_TASK_0                                           1
#define CONFIG_RAW_IDLE_EVENT                                       1
#define CONFIG_RAW_TASK_QUEUE_SIZE                                  1
#define CONFIG_RAW_TASK_SEMAPHORE                                   1

/*enable different task function*/
#define CONFIG_RAW_TASK_CREATE                                      1
#define CONFIG_RAW_TASK_STACK_CHECK                                 1
#define CONFIG_RAW_TASK_SLEEP                                       1
#define CONFIG_RAW_TASK_SUSPEND                                     1
#define CONFIG_RAW_TASK_RESUME                                      1
#define CONFIG_RAW_TASK_PRIORITY_CHANGE                             1  
#define CONFIG_RAW_TASK_DELETE                                      1
#define CONFIG_RAW_TASK_WAIT_ABORT                                  1
#define CONFIG_RAW_TICK_TASK                                        1

/*enable different semphore function*/

#define CONFIG_RAW_SEMAPHORE_DELETE                                 1
#define CONFIG_RAW_SEMAPHORE_SET                                    1


/*enable different mutex function*/

#define CONFIG_RAW_MUTEX_DELETE                                     1

/*enable different event function*/

#define CONFIG_RAW_EVENT_DELETE                                     1

/*enable different queue function*/

#define CONFIG_RAW_QUEUE_FLUSH                                      1
#define CONFIG_RAW_QUEUE_DELETE                                     1
#define CONFIG_RAW_QUEUE_GET_INFORMATION                            1

/*enable different queue buffer function*/

#define CONFIG_RAW_QUEUE_BUFFER_FLUSH                               1
#define CONFIG_RAW_QUEUE_BUFFER_DELETE                              1
#define CONFIG_RAW_QUEUE_BUFFER_GET_INFORMATION                     1

/*enable different queue size function*/

#define CONFIG_RAW_QUEUE_SIZE_FLUSH                                 1
#define	CONFIG_RAW_QUEUE_SIZE_DELETE                                1
#define CONFIG_RAW_QUEUE_SIZE_GET_INFORMATION                       1

/*enable different mqueue  function*/
#define	CONFIG_RAW_MQUEUE_DELETE									1
#define	CONFIG_RAW_MQUEUE_GET_INFORMATION							1
#define	CONFIG_RAW_MQUEUE_FLUSH										1

/*enable different timer function*/

#define CONFIG_RAW_TIMER_DELETE                                     1
#define CONFIG_RAW_TIMER_DEACTIVATE                                 1
#define CONFIG_RAW_TIMER_CHANGE                                     1
#define CONFIG_RAW_TIMER_ACTIVATE                                   1

/*enable different user hook function*/
#define CONFIG_RAW_USER_HOOK                                        1

/*enable different module check  function*/

#define RAW_SYSTEM_CHECK                                            0
#define RAW_TASK_FUNCTION_CHECK                                     1
#define RAW_SEMA_FUNCTION_CHECK                                     1
#define RAW_QUEUE_FUNCTION_CHECK                                    1
#define RAW_QUEUE_BUFFER_FUNCTION_CHECK                             1
#define RAW_QUEUE_SIZE_FUNCTION_CHECK                               1
#define RAW_MQUEUE_FUNCTION_CHECK                                   1
#define RAW_EVENT_FUNCTION_CHECK                                    1
#define RAW_MUTEX_FUNCTION_CHECK                                    1
#define RAW_TIMER_FUNCTION_CHECK                                    1
#define RAW_BLOCK_FUNCTION_CHECK                                    1
#define RAW_BYTE_FUNCTION_CHECK                                     1

/*Set idle task task size, adjust as you need*/
#define IDLE_STACK_SIZE                                             256


#if (CONFIG_RAW_TIMER > 0)
/*set timer task stack size, adjust as you need*/
#define TIMER_STACK_SIZE                                            256
#define TIMER_TASK_PRIORITY                                         5
/*Must be 2^n size!*/
#define TIMER_HEAD_NUMBERS                                          4

#endif

#if (CONFIG_RAW_TASK_0 > 0)

/*task 0 stack size*/
#define TASK_0_STACK_SIZE                                           256

/*Must be 2^n size!, such as 4, 8, 16,32, etc.......*/
#define MAX_TASK_EVENT                                              32

#if (CONFIG_RAW_ZERO_INTERRUPT > 0)

#define OBJECT_INT_MSG_SIZE                                         20

#endif

#endif

#if (CONFIG_RAW_TICK_TASK > 0)
#define TICK_TASK_STACK_SIZE                                        256
#define TICK_TASK_PRIORITY                                          1
#endif

/*Must be 2^n size!, such as 4, 8, 16,32, etc.......*/
#define TICK_HEAD_ARRAY                                             8

/*default  task time slice*/
#define TIME_SLICE_DEFAULT                                          50
/*allowed interrupted nested level*/
#define INT_NESTED_LEVEL                                            100


#define RAW_CONFIG_CPU_TIME                                         0
#define RAW_SCHE_LOCK_MEASURE_CHECK                                 0
#define RAW_CPU_INT_DIS_MEASURE_CHECK                               0


#if (CONFIG_RAW_PRIO_MAX >= 256)
#error  "CONFIG_RAW_PRIO_MAX must be <= 255"
#endif

#if ((CONFIG_RAW_QUEUE_SIZE == 0) && (CONFIG_RAW_TASK_QUEUE_SIZE >= 1))
#error  "you need enable CONFIG_RAW_QUEUE_SIZE as well."
#endif

#if ((CONFIG_RAW_SEMAPHORE == 0) && (CONFIG_RAW_TASK_SEMAPHORE >= 1))
#error  "you need enable CONFIG_RAW_SEMAPHORE as well."
#endif

#if ((CONFIG_RAW_TASK_0 == 0) && (CONFIG_RAW_ZERO_INTERRUPT >= 1))
#error  "doesn't support this option, please check your option"
#endif


#endif

