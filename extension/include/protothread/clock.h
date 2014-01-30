#ifndef CLOCK_H
#define CLOCK_H

clock_time_t clock_time(void);


unsigned int clock_seconds(void);

#define CLOCK_SECOND	RAW_TICKS_PER_SECOND

#endif

