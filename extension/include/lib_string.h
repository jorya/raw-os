/*
     raw os - Copyright (C)  Lingjun Chen(jorya_txj).

    This file is part of raw os.

    raw os is free software; you can redistribute it it under the terms of the 
    GNU General Public License as published by the Free Software Foundation; 
    either version 3 of the License, or  (at your option) any later version.

    raw os is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. if not, write email to jorya.txj@gmail.com
                                      ---

    A special exception to the LGPL can be applied should you wish to distribute
    a combined work that includes raw os, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

#ifndef LIB_STRING_H
#define  LIB_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef size_t
#define size_t  unsigned int
#endif

void *raw_memmove(void *dest, const void *src, size_t count);
int  raw_memcmp(const void *cs, const void *ct, size_t count);
char *raw_strcpy(char *dest, const char *src);
char *raw_strncpy(char *dest, const char *src, size_t count);
char *raw_strcat(char *dest, const char *src);
char *raw_strncat(char *dest, const char *src, size_t count);
size_t raw_strlen(const char *s);
int raw_strncmp(const char *cs, const char *ct, size_t count);
int raw_strcmp(const char *cs, const char *ct);
long int raw_strtol(const char *nptr, char **endptr, int base);
const unsigned char *search_conf(const unsigned char *cp, const unsigned char *name);

#ifdef __cplusplus
}
#endif


#endif
