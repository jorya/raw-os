#ifndef RAW_FF_H
#define RAW_FF_H

typedef struct _Files {
	int fileNo;
	int b;
} _FileType;


typedef _FileType raw_FILE;

typedef struct _EnvS {
	/* Each "Task" should at least have a set of std file handles */
	_FileType *_stdin;
	_FileType *_stdout;
	_FileType *_stderr;
} _EnvType;

extern _EnvType *_EnvPtr;
int fileno(raw_FILE *);


/* File handles */
#define raw_stdin		(_EnvPtr->_stdin)
#define raw_stdout		(_EnvPtr->_stdout)
#define raw_stderr		(_EnvPtr->_stderr)


raw_FILE *raw_fopen(const char *path, const char *mode);

RAW_PROCESSOR_UINT  raw_fread(void *ptr, RAW_PROCESSOR_UINT size, RAW_PROCESSOR_UINT nmemb, raw_FILE *stream);

RAW_PROCESSOR_UINT  raw_fwrite(const char *ptr, RAW_PROCESSOR_UINT size, RAW_PROCESSOR_UINT nmemb, raw_FILE *stream);

RAW_S32 raw_fclose(raw_FILE *fp);

RAW_S32 raw_ferror(raw_FILE *fp);

raw_FILE *raw_freopen(const char *pathpath, const char *mode, raw_FILE *stream);

RAW_PROCESSOR_INT raw_fseek(raw_FILE *stream, RAW_PROCESSOR_INT offset,  RAW_PROCESSOR_INT whence);

RAW_PROCESSOR_INT raw_getc(raw_FILE *stream);

int raw_feof(raw_FILE *stream);

long raw_ftell(raw_FILE *stream);


int raw_ungetc(int c, raw_FILE *stream);
	

int raw_fflush(raw_FILE *fp);

int raw_setvbuf(raw_FILE *stream, char *buf, int type, unsigned size);


#endif

