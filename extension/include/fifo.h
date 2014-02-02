#ifndef FIFO_H
#define FIFO_H

#define CONFIG_FIFO_DYNAMIC_ALLOC 0

struct raw_fifo {

	RAW_U32     in;
	RAW_U32     out;
	RAW_U32     mask;
	void        *data;
	RAW_U32     free_bytes;
	RAW_U32     size;
	
};



#define fifo_min(x, y) ((x) > (y)?(y):(x))
#define fifo_max(x, y) ((x) > (y)?(x):(y))

RAW_S8 fifo_init(struct raw_fifo *fifo, void *buffer, RAW_U32 size);


RAW_U32 fifo_in(struct raw_fifo *fifo,
		const void *buf, unsigned int len);

RAW_U32 fifo_out(struct raw_fifo *fifo,
		void *buf, RAW_U32 len);

RAW_U32 fifo_out_peek(struct raw_fifo *fifo,
		void *buf, RAW_U32 len);


RAW_U32 fifo_out_all(struct raw_fifo *fifo, void *buf);


#endif

