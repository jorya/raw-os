#ifndef FIFO_H
#define FIFO_H

#define CONFIG_FIFO_DYNAMIC_ALLOC 0

struct raw_fifo_lock {

	RAW_U32     in;
	RAW_U32     out;
	RAW_U32     mask;
	void        *data;
	RAW_U32     free_bytes;
	RAW_U32     size;
	
};



#define fifo_lock_min(x, y) ((x) > (y)?(y):(x))
#define fifo_lock_max(x, y) ((x) > (y)?(x):(y))

RAW_S8 fifo_lock_init(struct raw_fifo_lock *fifo, void *buffer, RAW_U32 size);


RAW_U32 fifo_lock_in(struct raw_fifo_lock *fifo,
		const void *buf, RAW_U32 len, void (*user_lock)(void), void  (*user_unlock)(void));


RAW_U32 fifo_lock_out(struct raw_fifo_lock *fifo, void *buf, RAW_U32 len, 
		void (*user_lock)(void), void (*user_unlock)(void));



RAW_U32 fifo_lock_out_peek(struct raw_fifo_lock *fifo,
		void *buf, RAW_U32 len, void (*user_lock)(void), void  (*user_unlock)(void));



RAW_U32 fifo_lock_out_all(struct raw_fifo_lock *fifo, void *buf, 
		void (*user_lock)(void), void (*user_unlock)(void));



#endif

