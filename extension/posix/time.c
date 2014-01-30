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


/* 	2012-8  Created by jorya_txj
  *	xxxxxx   please added here
  */


#include <raw_api.h>
#include <posix/pthread.h>

#define NANOSECOND_PER_SECOND	1000000000UL

unsigned int calculate_ticks(const struct timespec *time)
{
	unsigned int tick = 0;

	tick += time->tv_nsec * RAW_TICKS_PER_SECOND / NANOSECOND_PER_SECOND;
	tick += time->tv_sec *  RAW_TICKS_PER_SECOND;

	return tick;
}


int clock_getres  (clockid_t clockid, struct timespec *res)
{
	clockid = clockid;
	res = res;

	return 0;
}

int clock_gettime (clockid_t clockid, struct timespec *tp)
{
	clockid = clockid;
	tp->tv_sec = 0;
	tp->tv_nsec = 0;

	return 0;
}

int clock_settime (clockid_t clockid, const struct timespec *tp)
{
	clockid = clockid;
	tp = tp;

	return 0;
}

