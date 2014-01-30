#ifndef PTHREAD_H
#define PTHREAD_H


#ifdef __cplusplus
extern "C" {
#endif

typedef RAW_S32 clockid_t;
typedef RAW_S32 key_t;		/* Used for interprocess communication. */
typedef RAW_S32 pid_t;		/* Used for process IDs and process group IDs. */
typedef RAW_S32 ssize_t;	/* Used for a count of bytes or an error indication. */
typedef unsigned int size_t;


typedef RAW_S32 pthread_condattr_t;
typedef long pthread_mutexattr_t;
typedef struct pthread_struct *pthread_t;


#define __SIZEOF_PTHREAD_ATTR_T 36

#define PTHREAD_CREATE_JOINABLE		0x1
#define PTHREAD_CREATE_DETACHED		0x2


typedef struct
{
	int              detachstate;   
	int              schedpolicy;  

	int              inheritsched;  
	int              scope;       
	unsigned int     guardsize;   
	int              stackaddr_set;
	void             *stackaddr;  
	unsigned int     stacksize; 
	   
}pthread_attr_t;


typedef struct pthread_struct {
	
	RAW_TASK_OBJ     task_obj;
	RAW_SEMAPHORE    task_sem;
	
	RAW_U8           detachstate;
	pthread_attr_t   attr;
	void             *ret;
	
} pthread_struct;



struct timespec {
	long tv_sec;			
	long tv_nsec;			
};

struct timeval {
    RAW_S32    tv_sec;         
    RAW_S32    tv_usec;      
};



struct timezone {
  int tz_minuteswest;	/* minutes west of Greenwich */
  int tz_dsttime;	/* type of dst correction */
};


struct tm {
  int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
  int tm_min;			/* Minutes.	[0-59] */
  int tm_hour;			/* Hours.	[0-23] */
  int tm_mday;			/* Day.		[1-31] */
  int tm_mon;			/* Month.	[0-11] */
  int tm_year;			/* Year - 1900. */
  int tm_wday;			/* Day of week.	[0-6] */
  int tm_yday;			/* Days in year.[0-365]	*/
  int tm_isdst;			/* DST.		[-1/0/1]*/

  long int tm_gmtoff;		/* Seconds east of UTC.  */
  const char *tm_zone;		/* Timezone abbreviation.  */
};



typedef struct pthread_mutex_t {

	RAW_S32 attr;
	RAW_MUTEX mutex_lock;
	
} pthread_mutex_t;



typedef struct pthread_cond_t
{
	RAW_S32 attr;
	RAW_SEMAPHORE sem_lock;
	
} pthread_cond_t;




unsigned int calculate_ticks(const struct timespec *time);
int clock_gettime(clockid_t clockid, struct timespec *tp);

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void * (*start_routine)(void *), void *arg);


#ifdef __cplusplus
}
#endif


#endif


