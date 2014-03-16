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

/* 	2013-4  Created by jorya_txj
  *	xxxxxx   please added here
  */


#ifndef RAW_INTERNAL_TYPE_H
#define RAW_INTERNAL_TYPE_H


/*
    Be very careful here, you can modyfy the following code, if only you understand what yor are doing!

 */  

#define         RAW_INTERNAL_NO_WAIT 0u
#define         RAW_INTERNAL_WAIT_FOREVER 0xffffffffu /*32 bit value, if RAW_TICK_TYPE is 64 bit, you need change it to 64 bit*/


typedef RAW_U32 RAW_TICK_TYPE;  /*32 bit or 64 bit unsigned value*/
typedef RAW_U8  TASK_0_EVENT_TYPE; /*8 bit ,16 bit or 32 bit unsigned value*/
typedef RAW_U32 MSG_SIZE_TYPE; /*32 bit or 64 bit unsigned value*/
typedef RAW_U64 RAW_IDLE_COUNT_TYPE; /*32 bit or 64 bit unsigned value*/

#endif

