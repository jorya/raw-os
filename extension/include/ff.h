#ifndef RAW_FF_H
#define RAW_FF_H

typedef void raw_FILE;

#define raw_stdin   (void *)0x11
#define raw_stdout   (void *)0x12
#define raw_stderr   (void *)0x13

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

