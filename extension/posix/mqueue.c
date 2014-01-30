#include <raw_api.h>
#include <posix/pthread.h>
#include <posix/errno.h>
#include <posix/mqueue.h>
#include <mm/raw_page.h>
#include <mm/raw_malloc.h>
#include <lib_string.h>

#define DEFAULT_MQUEUE_SIZE 1000


mqd_t mq_open(const char *name, int oflag, ...)
{
	mqd_t mqueue;
	RAW_VOID **msg_start;
	
	mqueue = raw_malloc(sizeof(mqd_t));
	msg_start = raw_malloc(DEFAULT_MQUEUE_SIZE);
	
	raw_mq_init(&mqueue->mqueue_posix, "mqueue",  raw_malloc, raw_free, msg_start, DEFAULT_MQUEUE_SIZE);
	
	return 0;
}



ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio)
{
	RAW_VOID  *receive_addr;
	RAW_U32 receive_size;
	
	raw_mq_receive (&mqdes->mqueue_posix, (RAW_VOID  **)&receive_addr, &receive_size, msg_prio, RAW_NO_WAIT);

	if (msg_len <  receive_size) {
		return EMSGSIZE;
	}
	
	raw_memcpy(msg_ptr, receive_addr, receive_size);
	
	return 0;
}



int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio)
{
	RAW_U16 ret;
	
	ret = raw_mq_send(&mqdes->mqueue_posix, (RAW_VOID  *)msg_ptr, msg_len, msg_prio);
	
	if (ret == RAW_BLOCK_TIMEOUT) {

		return ETIMEDOUT;
	}

	return 0;
}


ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
	unsigned *msg_prio, const struct timespec *abs_timeout)
{
	RAW_U32 ticks;
	RAW_VOID  *receive_addr;
	RAW_U32 receive_size;
	RAW_U16 ret;
	
	ticks = calculate_ticks(abs_timeout);
	
	ret = raw_mq_receive (&mqdes->mqueue_posix, (RAW_VOID  **)&receive_addr, &receive_size, msg_prio, ticks);

	if (msg_len <  receive_size) {
		return EMSGSIZE;
	}

	if (ret == RAW_BLOCK_TIMEOUT) {

		return ETIMEDOUT;
	}
	
	raw_memcpy(msg_ptr, receive_addr, receive_size);
	
	return 0;
	
}


int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio,
		const struct timespec *abs_timeout)
{
	RAW_U16 ret;
	
	ret = raw_mq_send(&mqdes->mqueue_posix, (RAW_VOID *)msg_ptr, msg_len, msg_prio);
	
	if (ret != RAW_SUCCESS) {

		RAW_ASSERT(0);
	}


	return 0;
	
}


int mq_close(mqd_t mqdes)
{
	
	raw_mqueue_delete(&mqdes->mqueue_posix);

	return 0;
}

