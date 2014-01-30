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



#ifndef RAW_STM_H
#define RAW_STM_H


typedef RAW_U16 STM_SIGNAL;


typedef struct STATE_EVENT {

	RAW_U16     sig;
	RAW_U8      ref_count;
	RAW_VOID    *which_pool; 
	
	
	  
	
} STATE_EVENT;


typedef RAW_U16 (*stm_state_handler)(void *me, STATE_EVENT *e);


typedef struct STM_STRUCT {
	
    stm_state_handler state;        
    stm_state_handler temp; 
	
} STM_STRUCT;

#define STM_SUCCESS 0


#define STM_STATE_CAST(handler)  ((stm_state_handler)(handler))


#define STM_RET_HANDLED         1


#define STM_RET_IGNORED         2


#define STM_RET_TRAN            3


#define STM_TRAN(state)        (((STM_STRUCT *)me)->temp = STM_STATE_CAST(state), STM_RET_TRAN)


#define STM_RET_FATHER          4


#define STM_FATHER(father)      (((STM_STRUCT *)me)->temp = STM_STATE_CAST(father),  STM_RET_FATHER)



#define STM_RET_UNHANDLED       5



#define STM_EMPTY_SIG           0
#define STM_MAX_NEST_DEPTH      6


enum RAW_Reserved_Signals {

    STM_ENTRY_SIG = 1,                  
    STM_EXIT_SIG,                        
    STM_INIT_SIG,
    STM_TIMEOUT_SIG,                        
    STM_USER_SIG     
};



extern STATE_EVENT STM_GLOBAL_EVENT[4];

#define STM_TRIG(state, sig) ((*(state))(me, &STM_GLOBAL_EVENT[sig]))



#define FSM_CONSTRUCTOR(me, initial) do { \
    (me)->state = 0; \
    (me)->temp  = (initial); \
} while (0)


RAW_U16 hsm_top(void  *me, STATE_EVENT *e);


#define HSM_CONSTRUCTOR(me, initial) do { \
    (me)->state = hsm_top; \
    (me)->temp  = (initial); \
} while (0)


#define STM_ENTER(state) STM_TRIG((state), STM_ENTRY_SIG)

#define STM_EXIT(state)  STM_TRIG((state), STM_EXIT_SIG)

RAW_U16 fsm_init(STM_STRUCT *me, STATE_EVENT *e); 
void fsm_exceute(STM_STRUCT *me, STATE_EVENT *e); 

void hsm_init(STM_STRUCT *me, STATE_EVENT *e);
void hsm_exceute(STM_STRUCT *me, STATE_EVENT *e);

RAW_U16 is_hsm_in_state(STM_STRUCT *me, stm_state_handler state);

#endif

