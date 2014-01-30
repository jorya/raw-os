/*
 * Copyright (c) 2004-2005, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */



#include <protothread/protothread.h>


/*
 * Pointer to the currently running process structure.
 */
struct process *process_list = 0;
struct process *process_current = 0;
 
static process_event_t lastevent;

/*
 * Structure used for keeping the queue of active events.
 */
struct event_data {
  process_event_t ev;
  process_data_t data;
  struct process *p;
};

static process_num_events_t nevents, fevent;
static struct event_data events[PROCESS_CONF_NUMEVENTS];


process_num_events_t process_maxevents;


static volatile unsigned char poll_requested;

#define PROCESS_STATE_NONE        0
#define PROCESS_STATE_RUNNING     1
#define PROCESS_STATE_CALLED      2

static void call_process(struct process *p, process_event_t ev, process_data_t data);



/*---------------------------------------------------------------------------*/
process_event_t
process_alloc_event(void)
{
  return lastevent++;
}



/*not called by isr*/
void process_start(struct process *p, const char *arg)
{
  struct process *q;

  /* First make sure that we don't try to start a process that is
     already running. */
  for(q = process_list; q != p && q != 0; q = q->next);

  /* If we found the process on the process list, we bail out. */
  if(q == p) {
    return;
  }
  /* Put on the procs list.*/
  p->next = process_list;
  process_list = p;
  p->state = PROCESS_STATE_RUNNING;
  PT_INIT(&p->pt);

  /* Post a synchronous initialization event to the process. */
  process_post_synch(p, PROCESS_EVENT_INIT, (process_data_t)arg);
}


/*not called by isr*/
static void exit_process(struct process *p, struct process *fromprocess)
{
  register struct process *q;
  struct process *old_current = process_current;


  /* Make sure the process is in the process list before we try to
     exit it. */
  for(q = process_list; q != p && q != 0; q = q->next);
  if(q == 0) {
    return;
  }

  if(process_is_running(p)) {
    /* Process was running */
    p->state = PROCESS_STATE_NONE;

    /*
     * Post a synchronous event to all processes to inform them that
     * this process is about to exit. This will allow services to
     * deallocate state associated with this process.
     */
    for(q = process_list; q != 0; q = q->next) {
      if(p != q) {
	call_process(q, PROCESS_EVENT_EXITED, (process_data_t)p);
      }
    }

    if(p->thread != 0 && p != fromprocess) {
      /* Post the exit event to the process that is about to exit. */
      process_current = p;
      p->thread(&p->pt, PROCESS_EVENT_EXIT, 0);
    }
  }

  if(p == process_list) {
    process_list = process_list->next;
  } else {
    for(q = process_list; q != 0; q = q->next) {
      if(q->next == p) {
	q->next = p->next;
	break;
      }
    }
  }

  process_current = old_current;
}
/*---------------------------------------------------------------------------*/
static void call_process(struct process *p, process_event_t ev, process_data_t data)
{
  int ret;


  
  if((p->state & PROCESS_STATE_RUNNING) &&
     p->thread != 0) {
   
    process_current = p;
    p->state = PROCESS_STATE_CALLED;
    ret = p->thread(&p->pt, ev, data);
    if(ret == PT_EXITED ||
       ret == PT_ENDED ||
       ev == PROCESS_EVENT_EXIT) {
      exit_process(p, p);
    } else {
      p->state = PROCESS_STATE_RUNNING;
    }
  }
}
/*---------------------------------------------------------------------------*/
void
process_exit(struct process *p)
{
  exit_process(p, PROCESS_CURRENT());
}
/*---------------------------------------------------------------------------*/
void
process_init(void)
{
  lastevent = PROCESS_EVENT_MAX;

  nevents = fevent = 0;
  process_maxevents = 0;


  process_current = process_list = 0;
}
/*---------------------------------------------------------------------------*/
/*
 * Call each process' poll handler.
 */

static void do_poll(void)
{
	struct process *p;
	RAW_SR_ALLOC();

	RAW_CRITICAL_ENTER();
	poll_requested--;
	RAW_CRITICAL_EXIT();
	/* Call the processes that needs to be polled. */
	for(p = process_list; p != 0; p = p->next) {
		
		if(p->needspoll) {
			p->state = PROCESS_STATE_RUNNING;
			p->needspoll = 0;
			call_process(p, PROCESS_EVENT_POLL, 0);
		}
	}
}

/*---------------------------------------------------------------------------*/
/*
 * Process the next event in the event queue and deliver it to
 * listening processes.
 */
/*---------------------------------------------------------------------------*/
static void
do_event(void)
{
  static process_event_t ev;
  static process_data_t data;
  static struct process *receiver;
  static struct process *p;

  RAW_SR_ALLOC();
  
  /*
   * If there are any events in the queue, take the first one and walk
   * through the list of processes to see if the event should be
   * delivered to any of them. If so, we call the event handler
   * function for the process. We only process one event at a time and
   * call the poll handlers inbetween.
   */

  RAW_CRITICAL_ENTER();
  if (nevents > 0) {
    
    /* There are events that we should deliver. */
    ev = events[fevent].ev;
    
    data = events[fevent].data;
    receiver = events[fevent].p;

    /* Since we have seen the new event, we move pointer upwards
       and decrese the number of events. */
    fevent = (fevent + 1) & (PROCESS_CONF_NUMEVENTS - 1);
    --nevents;
	RAW_CRITICAL_EXIT();

    /* If this is a broadcast event, we deliver it to all events, in
       order of their priority. */
    if(receiver == PROCESS_BROADCAST) {
      for(p = process_list; p != 0; p = p->next) {

	/* If we have been requested to poll a process, we do this in
	   between processing the broadcast event. */
	if(poll_requested) {
	  do_poll();
	}
	call_process(p, ev, data);
      }
    } else {
      /* This is not a broadcast event, so we deliver it to the
	 specified process. */
      /* If the event was an INIT event, we should also update the
	 state of the process. */
      if(ev == PROCESS_EVENT_INIT) {
	receiver->state = PROCESS_STATE_RUNNING;
      }

      /* Make sure that the process actually is running. */
      call_process(receiver, ev, data);
    }
  }

  else {

	RAW_CRITICAL_EXIT();

  }

}
/*---------------------------------------------------------------------------*/
int
process_run(void)
{
  /* Process poll events. */
  if(poll_requested) {
    do_poll();
  }

  /* Process one event from the queue */
  do_event();

  return nevents + poll_requested;
}
/*---------------------------------------------------------------------------*/
int
process_nevents(void)
{
  return nevents + poll_requested;
}

/*this function can be used in interrupt*/
int
process_post(struct process *p, process_event_t ev, process_data_t data)
{
  static process_num_events_t snum;
  RAW_SR_ALLOC();
  
  if(nevents == PROCESS_CONF_NUMEVENTS) {

    RAW_ASSERT(0);
  }

  RAW_CRITICAL_ENTER();
  
  snum = (process_num_events_t)(fevent + nevents) & (PROCESS_CONF_NUMEVENTS - 1);
  ++nevents;
  
  RAW_CRITICAL_EXIT();
  
  events[snum].ev = ev;
  events[snum].data = data;
  events[snum].p = p;
  
  if(nevents > process_maxevents) {
    process_maxevents = nevents;
  }

  
  return PROCESS_ERR_OK;
}



/*Should not used in interrupt*/
RAW_U16 process_post_synch(struct process *p, process_event_t ev, process_data_t data)
{
	struct process *caller = process_current;

	if (raw_int_nesting) {

		return RAW_NOT_CALLED_BY_ISR;

	}

	call_process(p, ev, data);
	process_current = caller;
	
	return RAW_SUCCESS;
}

void process_poll(struct process *p)
{
	RAW_SR_ALLOC();
	
	if (p) {
		
		if(p->state == PROCESS_STATE_RUNNING ||
			p->state == PROCESS_STATE_CALLED) {
			
			RAW_CRITICAL_ENTER();
			p->needspoll = 1;
			poll_requested++;
			RAW_CRITICAL_EXIT();
		}
	}
}
/*---------------------------------------------------------------------------*/
int
process_is_running(struct process *p)
{
  return p->state != PROCESS_STATE_NONE;
}


