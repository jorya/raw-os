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


#include <raw_api.h>

#if (CONFIG_RAW_IDLE_EVENT > 0)

/*
************************************************************************************************************************
*                                   The top of the hsm state machine
*
* Description: This function is used to init the hsm state machine to state hsm_top.
*
* Arguments  :me (unused)
*                    ---------
*                    e (unused)   
*				         
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 hsm_top(void  *me, STATE_EVENT *e) 
{
    me = me;       
    e = e; 
	
    return STM_RET_IGNORED;                 
}



/*
************************************************************************************************************************
*                                   Init the finit state machine
*
* Description: This function is used to init the finit state machine.
*
* Arguments  :me is the state machine
*                    ---------
*                    e is the trig event   
*				         
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
RAW_U16 fsm_init(STM_STRUCT *me, STATE_EVENT *e) 
{
    RAW_U16 ret;
	
    if (me->temp == 0) {

		RAW_ASSERT(0);

    }

	/*do the fsm constructor init function*/
	ret = (*me->temp)(me, e);

	/*transition must happen here*/
	if (ret != STM_RET_TRAN) {
		
		RAW_ASSERT(0);
	}

	/*trig the STM_ENTRY_SIG to the new transioned state*/
    STM_TRIG(me->temp, STM_ENTRY_SIG);

	/*change to new state*/
    me->state = me->temp; 

	return STM_SUCCESS;

   
}


/*
************************************************************************************************************************
*                                   exceute the finit state machine
*
* Description: This function is used to exceute the finit state machine.
*
* Arguments  :me is the state machine
*                    ---------
*                    e is the trig event 
*				         
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
void fsm_exceute(STM_STRUCT *me, STATE_EVENT *e) 
{
	RAW_U16 ret;

	/*State must be stable here*/
	if (me->state != me->temp) {

		RAW_ASSERT(0);

	}

	/*exceute the state function with new event*/
    ret = (*me->state)(me, e); 
	
    if (ret == STM_RET_TRAN) {                            

		/*exit the original state */
		STM_EXIT(me->state); 
		/*enter the new state*/
		STM_ENTER(me->temp); 
		/*change to new state*/
		me->state = me->temp; 
    }
   
}



/*
************************************************************************************************************************
*                                   Init the hsm state machine
*
* Description: This function is used to init the finit state machine.
*
* Arguments  :me is the state machine
*                    ---------
*                    e is the trig event   
*				         
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
void hsm_init(STM_STRUCT *me, STATE_EVENT *e)
{
	RAW_U16 ret;
	RAW_S8 ip;

	/*Max nested state levels*/
	stm_state_handler path[STM_MAX_NEST_DEPTH];
	        
	stm_state_handler t = me->state;

	if (me->temp == 0) {

		RAW_ASSERT(0);

	}


	/*if state is not equal to the hsm top state, just assert*/
	if (t != STM_STATE_CAST(hsm_top)) {

		RAW_ASSERT(0);
	}



	/*do the hsm constructor init function*/
	ret = (*me->temp)(me, e);

	/*transition must happen here*/
	if (ret != STM_RET_TRAN) {

		RAW_ASSERT(0);
	}


	/*Becareful STM_INIT_SIG must trig t state to the nested children state, otherwise hsm crash*/
	do { 
		
		ip = 0;
		
		path[0] = me->temp;

		/*Find all the father state until to hsm_top*/
		STM_TRIG(me->temp, STM_EMPTY_SIG);
		
		while (me->temp != t) {
			
			++ip;
			path[ip] = me->temp;
			STM_TRIG(me->temp, STM_EMPTY_SIG);
		}
		
		me->temp = path[0];
		                               
		if (ip >= STM_MAX_NEST_DEPTH) {
			
			RAW_ASSERT(0);

		}

		/*trig STM_ENTRY_SIG from father source state to nested children state*/
		do {        
			STM_ENTER(path[ip]);                         
			--ip;
		} while (ip >= 0);

		t = path[0];  
		/*trig the STM_INIT_SIG to the new transitioned state, if new transion happened again, then we need do int init again*/
	} while (STM_TRIG(t, STM_INIT_SIG) == STM_RET_TRAN);


	/*change to new state*/
	me->state = t;                      
	me->temp  = t;                     

    
}



/*
************************************************************************************************************************
*                                   Exceute the hsm state machine
*
* Description: This function is used to exceute the hsm state machine.
*
* Arguments  :me is the state machine
*                    ---------
*                    e is the trig event   
*				         
* Returns			
*						
* Note(s)    	
*
*             
************************************************************************************************************************
*/
void hsm_exceute(STM_STRUCT *me, STATE_EVENT *e)
{
	stm_state_handler s;
	RAW_U16 r;
	RAW_S8 ip;
	RAW_S8 iq;
	
	stm_state_handler path[STM_MAX_NEST_DEPTH];
	
    stm_state_handler t = me->state;

 	/*state must be stable here*/
	if (t != me->temp) {

		RAW_ASSERT(0);
	}

    
    do {                             
        s = me->temp;

		/*exceute the state function with new event*/
        r = (*s)(me, e);                         

        if (r == STM_RET_UNHANDLED) {           
			/*Move up to father state*/
            r = STM_TRIG(s, STM_EMPTY_SIG);       
        }

	/*move up to the father state to find suitable state to handle the sig*/
    } while (r == STM_RET_FATHER);

	/*if state transition happened then process it*/
    if (r == STM_RET_TRAN) {                            
       
        ip = -1;  
		
		/*save the transitioned state*/
        path[0] = me->temp;            
        path[1] = t;

		/*t is the source state, and s is the state which cause new state change*/
		/*for example s is the father state of t*/
        while (t != s) {  

			/*if STM_EXIT_SIG is handled, trig STM_EMPTY_SIG to find the father state*/ 
			/*if STM_EXIT_SIG not handled , then me->temp hold the father state*/
            if (STM_TRIG(t, STM_EXIT_SIG) == STM_RET_HANDLED) {
                
            	STM_TRIG(t, STM_EMPTY_SIG);
            }

			/*move t to one father state up*/
            t = me->temp;                 
        }

		/*t is the target transition state*/
        t = path[0];                           

		/*all the following code is try to find the LCA and exit from the source state to LCA state*/
		/*Be careful LCA state is either not entered not exited.*/
		/*all the father state of the target transition state is stored to path from hight to low etc, path[0] is the target transition state*/
		
        if (s == t) {      
            STM_EXIT(s);                                
            ip = 0;                            
        }
        else {
        	STM_TRIG(t, STM_EMPTY_SIG);     

            t = me->temp;
            if (s == t) {                
                ip = 0;                        
            }
			
            else {
            	STM_TRIG(s, STM_EMPTY_SIG);    
                
                if (me->temp == t) {
                    STM_EXIT(s);                        
                    ip = 0;                   
                }
				
                else {
                                         
                    if (me->temp == path[0]) {
                        STM_EXIT(s);                    
                    }
					
                    else {
                        iq = 0;      
                        ip = 1;  
                        path[1] = t;      
                        t = me->temp;                 
                                              
                        r = STM_TRIG(path[1], STM_EMPTY_SIG);
											   
                        while (r == STM_RET_FATHER) {
                            ++ip;
                            path[ip] = me->temp;    
                            if (me->temp == s) {      
                                iq = 1;  
                                            
								if (ip >= STM_MAX_NEST_DEPTH) {

									RAW_ASSERT(0);

								}
								
                                --ip;           
                                r = STM_RET_HANDLED;    
                            }
                            else {  
                                r = STM_TRIG(me->temp, STM_EMPTY_SIG);
                            }
                        }
						
                        if (iq == 0) {   
             
							if (ip >= STM_MAX_NEST_DEPTH) {

								RAW_ASSERT(0);

							}

                            STM_EXIT(s);               

                     
                            iq = ip;
                            r = STM_RET_IGNORED;    
                            do {
                                if (t == path[iq]) {   
                                    r = STM_RET_HANDLED;
                                                         
                                    ip = iq - 1;
                                    iq = -1;
                                }
                                else {
                                    --iq; 
                                }
                            } while (iq >= 0);

                            if (r != STM_RET_HANDLED) { 
                                   
                                r = STM_RET_IGNORED;         
                                do {
                                                      
                                    if (STM_TRIG(t, STM_EXIT_SIG) == STM_RET_HANDLED) {
										STM_TRIG(t, STM_EMPTY_SIG);
                                    }
									
                                    t = me->temp;    
                                    iq = ip;
                                    do {
                                        if (t == path[iq]) {
                                                       
                                            ip = iq - 1;
                                            iq = -1;
                                            r = STM_RET_HANDLED;
                                        }
                                        else {
                                            --iq;
                                        }
                                    } while (iq >= 0);
                                } while (r != STM_RET_HANDLED);
                            }
                        }
                    }
                }
            }
        }

		/*trig STM_ENTRY_SIG from LCA to transioned state*/
        for (; ip >= 0; --ip) {
            STM_ENTER(path[ip]);                        
        }
		
        t = path[0];                     
        me->temp = t;                            

		/*trig the STM_INIT_SIG to the new transitioned state, if new transion happened again, then we need do it again*/
		/*Becareful STM_INIT_SIG must trig t state to the nested children state, otherwise hsm crash*/
        while (STM_TRIG(t, STM_INIT_SIG) == STM_RET_TRAN) {

			ip = 0;
			path[0] = me->temp;

			/*Find all the father state until to source t state */
			STM_TRIG(me->temp, STM_EMPTY_SIG);   

			while (me->temp != t) {
				++ip;
				path[ip] = me->temp;
				STM_TRIG(me->temp, STM_EMPTY_SIG);
			}

			me->temp = path[0];
			                            
			if (ip >= STM_MAX_NEST_DEPTH) {

				RAW_ASSERT(0);

			}

			/*trig STM_ENTRY_SIG from father source state to nested transition children state*/
			do {   
				STM_ENTER(path[ip]);                     
				--ip;
			} while (ip >= 0);

			/*remember the target transitoned state*/
			t = path[0];
			
		}


	}
   
	/*change to new state*/
	me->state = t;                       
	me->temp  = t; 
	
}


/*
************************************************************************************************************************
*                                   Test whether the current state is in state or not
* Description: This function is used to test whether the current state is in state or not.
*
* Arguments  :me is the state machine
*                    ---------
*                    state is to compared with currenet state.
*				         
* Returns			
*						
* Note(s)        if the state is the father state of current state, it also return 1.	
*
*             
************************************************************************************************************************
*/
RAW_U16 is_hsm_in_state(STM_STRUCT *me, stm_state_handler state) 
{
    RAW_U16 inState = 0; 
    RAW_U16 r;

	RAW_ASSERT(me->temp == me->state);

    do {
        if (me->temp == state) {                    
            inState = 1;              
            break;                   
        }
		
        else {
            r = STM_TRIG(me->temp, STM_EMPTY_SIG);
        }
		
    } while (r != STM_RET_IGNORED); 
	
    me->temp = me->state;        

    return inState;                                   
}


#endif


