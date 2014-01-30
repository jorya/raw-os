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



/* 	
 *	2012-9  Slab Memory Managment
 *          by Hu Zhigang <huzhigang.rawos@gmail.com>
 */



#ifndef KERN_CONFIG_H_
#define KERN_CONFIG_H_


#define STACK_SIZE  PAGE_SIZE

#define CONFIG_INIT_TASKS        32
#define CONFIG_TASK_NAME_BUFLEN  32


typedef unsigned int	uintptr_t;
typedef unsigned int	pfn_t;
typedef unsigned int	sysarg_t;
typedef signed	 int	native_t;
typedef unsigned int	atomic_count_t;


#define raw_printk   vc_port_printf

typedef struct {
	uintptr_t addr;
	size_t size;
	char name[CONFIG_TASK_NAME_BUFLEN];
} init_task_t;

typedef struct {
	size_t cnt;
	init_task_t tasks[CONFIG_INIT_TASKS];
} init_t;

/** Boot allocations.
 *
 * Allocatations made by the boot that are meant to be used by the kernel
 * are all recorded in the ballocs_t type.
 */
typedef struct {
	uintptr_t base;
	size_t size;
} ballocs_t;

typedef struct {
	uintptr_t base;
	size_t kernel_size;          /**< Size of memory in bytes taken by kernel and stack */
	
	uintptr_t stack_base;        /**< Base adddress of initial stack */
	size_t stack_size;           /**< Size of initial stack */
} config_t;


#endif

/** @}
 */
