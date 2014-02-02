#include <raw_api.h>
#include <fifo_lock.h>


/*
 * internal helper to calculate the unused elements in a fifo
 */
static RAW_U32 fifo_unused(struct raw_fifo_lock *fifo)
{
	return (fifo->mask + 1) - (fifo->in - fifo->out);
}



static RAW_S8 is_power_of_2(RAW_U32 n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}


/*static fifio alloc do not need raw_malloc*/

RAW_S8 fifo_lock_init(struct raw_fifo_lock *fifo, void *buffer, RAW_U32 size)
{
	/*
	 * round down to the next power of 2, since our 'let the indices
	 * wrap' technique works only in this case.
	 */
	if (!is_power_of_2(size)) {

		RAW_ASSERT(0);
	}

	fifo->in = 0;
	fifo->out = 0;
	fifo->data = buffer;

	if (size < 2) {
		fifo->mask = 0;
		return 1;
	}
	
	fifo->mask = size - 1;
	fifo->free_bytes = size;
	
	return 0;
}


static void fifo_copy_in(struct raw_fifo_lock *fifo, const void *src,
		RAW_U32 len, RAW_U32 off)
{
	RAW_U32 l;
	
	RAW_U32 size = fifo->mask + 1;
	
	off &= fifo->mask;
	
	l = fifo_lock_min(len, size - off);

	raw_memcpy((unsigned  char *)fifo->data + off, src, l);
	raw_memcpy(fifo->data, (unsigned  char *)src + l, len - l);

	
}

/*fifo write in*/
RAW_U32 fifo_lock_in(struct raw_fifo_lock *fifo,
		const void *buf, RAW_U32 len, void (*user_lock)(void), void  (*user_unlock)(void))
{
	RAW_U32 l;

	user_lock();
	
	l = fifo_unused(fifo);
	if (len > l)
		len = l;

	fifo_copy_in(fifo, buf, len, fifo->in);
	fifo->in += len;

	fifo->free_bytes -= len;

	user_unlock();
	
	return len;
}

static void kfifo_copy_out(struct raw_fifo_lock *fifo, void *dst,
		RAW_U32 len, RAW_U32 off)
{
	RAW_U32 l;
	RAW_U32 size = fifo->mask + 1;
	
	off &= fifo->mask;
	
	l = fifo_lock_min(len, size - off);

	raw_memcpy(dst, (unsigned  char *)fifo->data + off, l);
	raw_memcpy((unsigned  char *)dst + l, fifo->data, len - l);
	
}

static RAW_U32 internal_fifo_out_peek(struct raw_fifo_lock *fifo,
		void *buf, RAW_U32 len)
{
	RAW_U32 l;

	l = fifo->in - fifo->out;
	if (len > l)
		len = l;

	kfifo_copy_out(fifo, buf, len, fifo->out);
	return len;
}

/*fifo read out but data remain in fifo*/

RAW_U32 fifo_lock_out_peek(struct raw_fifo_lock *fifo,
		void *buf, RAW_U32 len, void (*user_lock)(void), void  (*user_unlock)(void))
{

	RAW_U32 ret_len;

	user_lock();

	ret_len = internal_fifo_out_peek(fifo, buf, len);
	
	user_unlock();

	return ret_len;

}

/*fifo read out*/

RAW_U32 fifo_lock_out(struct raw_fifo_lock *fifo, void *buf, RAW_U32 len, void (*user_lock)(void), void (*user_unlock)(void))
{
	user_lock();
	
	len = internal_fifo_out_peek(fifo, buf, len);
	fifo->out += len;

	fifo->free_bytes += len;
	
	user_unlock();
	
	return len;
}

/*fifo read out all*/

RAW_U32 fifo_lock_out_all(struct raw_fifo_lock *fifo, void *buf, void (*user_lock)(void), void (*user_unlock)(void))
{
	RAW_U32 len;
	
	user_lock();

	len = fifo->size - fifo->free_bytes;

	if (len == 0) {

		user_unlock();
		return 0;
	}

	len = internal_fifo_out_peek(fifo, buf, len);
	fifo->out += len;

	fifo->free_bytes += len;
	
	user_unlock();
	
	return len;
	
}

