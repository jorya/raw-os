#include <raw_api.h>
#include <posix/pthread.h>
#include <posix/errno.h>
#include <mm/raw_page.h>
#include <mm/raw_malloc.h>
#include <posix/semphore.h>



int sem_close(sem_t *sem)
{
	

    return 0;
}

int sem_destroy(sem_t *sem)
{
	
    raw_semaphore_delete(&sem->sem_posix);

    return 0;
}

int sem_unlink(const char *name)
{
	
    return 0;
}

int sem_getvalue(sem_t *sem, int *sval)
{

	*sval = (int)(sem->sem_posix.count);
	
	return 0;
}

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	
	raw_semaphore_create(&sem->sem_posix ,"sem", value);
	return 0;
}

sem_t *sem_open(const char *name, int oflag, ...)
{

	return 0;	
}



int sem_post(sem_t *sem)
{
	raw_semaphore_put(&sem->sem_posix);
	return 0;	
}



int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
{
	RAW_U32 ticks;
	
	ticks = calculate_ticks(abs_timeout);
	raw_semaphore_get(&sem->sem_posix, ticks);
	return 0;	
}

int sem_trywait(sem_t *sem)
{
	RAW_U16 ret;

	ret = raw_semaphore_get(&sem->sem_posix, RAW_NO_WAIT);

	if (ret == RAW_NO_PEND_WAIT) {
		return EAGAIN;
	}

	return 0;	
}


int sem_wait(sem_t *sem)
{
	raw_semaphore_get(&sem->sem_posix, RAW_WAIT_FOREVER);
	return 0;	
}

