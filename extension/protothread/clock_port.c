#include <protothread/protothread.h>

clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = RAW_TICKS_PER_SECOND;


void proto_tick_handler()
{
  
	current_clock++;

	if(etimer_pending() && etimer_next_expiration_time() <= current_clock) {
		
		etimer_request_poll();

	}
	
	if (--second_countdown == 0) {
		
		current_seconds++;
		second_countdown = RAW_TICKS_PER_SECOND;
	}
}


void clock_init()
{
  
}

clock_time_t clock_time(void)
{
  return current_clock;
}



unsigned int clock_seconds(void)
{

  return current_seconds;
  
}


