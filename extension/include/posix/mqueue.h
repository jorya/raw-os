#ifndef MQUEUE_H
#define MQUEUE_H



struct mqd
{
	RAW_MQUEUE mqueue_posix;	
};

typedef struct mqd *mqd_t;


#endif

