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



#include <lib_string.h>


/*** macros ***/
/* text evaluation and conversion macro */
#define _TO_UPPER_CASE(c) \
        ( (((c) >= 'a') && ((c) <= 'z')) ? ((c) + ('A' - 'a')) : (c) )
#define _IS_SPACE_CODE(c) \
        ( ((c) == ' ') || ((c) == '\t') )



/**
 * memmove - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * Unlike memcpy(), memmove() copes with overlapping areas.
 */
void *raw_memmove(void *dest, const void *src, size_t count)
{
	char *tmp;
	const char *s;

	if (dest <= src) {
		tmp = dest;
		s = src;
		while (count--)
			*tmp++ = *s++;
	} else {
		tmp = dest;
		tmp += count;
		s = src;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}



/**
 * memcmp - Compare two areas of memory
 * @cs: One area of memory
 * @ct: Another area of memory
 * @count: The size of the area.
 */
int raw_memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}


/**
 * strcpy - Copy a %NUL terminated string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 */
char *raw_strcpy(char *dest, const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}


/**
 * strncpy - Copy a length-limited, %NUL-terminated string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 * @count: The maximum number of bytes to copy
 *
 * The result is not %NUL-terminated if the source exceeds
 * @count bytes.
 *
 * In the case where the length of @src is less than  that  of
 * count, the remainder of @dest will be padded with %NUL.
 *
 */
char *raw_strncpy(char *dest, const char *src, size_t count)
{
	char *tmp = dest;

	while (count) {
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		count--;
	}
	return dest;
}


char *raw_strcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}


/**
 * strncat - Append a length-limited, %NUL-terminated string to another
 * @dest: The string to be appended to
 * @src: The string to append to it
 * @count: The maximum numbers of bytes to copy
 *
 * Note that in contrast to strncpy(), strncat() ensures the result is
 * terminated.
 */
char *raw_strncat(char *dest, const char *src, size_t count)
{
	char *tmp = dest;

	if (count) {
		while (*dest)
			dest++;
		while ((*dest++ = *src++) != 0) {
			if (--count == 0) {
				*dest = '\0';
				break;
			}
		}
	}
	return tmp;
}


/**
 * strlen - Find the length of a string
 * @s: The string to be sized
 */
size_t raw_strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}



/**
 * strncmp - Compare two length-limited strings
 * @cs: One string
 * @ct: Another string
 * @count: The maximum number of bytes to compare
 */
int raw_strncmp(const char *cs, const char *ct, size_t count)
{
	unsigned char c1, c2;

	while (count) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
		count--;
	}
	return 0;
}


/**
 * strcmp - Compare two strings
 * @cs: One string
 * @ct: Another string
 */
int raw_strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}


/* strtol : convert text string to integer value (long int) */
long int raw_strtol(const char *nptr, char **endptr, int base)
{
	const char *num_table = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	long int value = 0;
	int sign = 1, i;
	char *cp;

	while (_IS_SPACE_CODE(*nptr)) {
		++nptr;
	}

	switch (*nptr) {
	  case '-':
		sign = -1;
		/* no break */
	  case '+':
		++nptr;
		/* no break */
	  default:
		break;
	}

	if (base == 16) {
		if (*nptr == '0') {
			++nptr;
			if (_TO_UPPER_CASE(*nptr) != 'X') {
				goto PARSE_START;
			}
			++nptr;
		}
	}

	if (base == 0) {
		if (*nptr == '0') {
			++nptr;
			if (_TO_UPPER_CASE(*nptr) == 'X') {
				++nptr;
				base = 16;
			} else {
				base = 8;
			}
		} else {
			base = 10;
		}
	}

	if ((base < 2) || (base > 36)) {
		base = 10;
	}

PARSE_START:
	while (*nptr != '\0') {
		cp = (char *)num_table;
		for (i = 0; i < base; ++i) {
			if (_TO_UPPER_CASE(*nptr) == *cp) {
				break;
			}
			++cp;
		}
		if (i >= base) {
			goto PARSE_END;
		}

		value = value * base + i;
		++nptr;
	}

PARSE_END:
	if (endptr != 0) {
		*endptr = (char *)nptr;
	}
	return value * sign;
}


/* ------------------------------------------------------------------------ */
/*
 *	System information management function
 */

/*
 * Search 'name' information
 *	If it is not found, return NULL.
 */
const unsigned char *search_conf(const unsigned char *cp, const unsigned char *name)
{
	size_t		len = raw_strlen((char*)name);
	const unsigned char	*p;

	while ( *cp != '\0' ) {
		if ( *cp == name[0] ) {
			for ( p = cp; *p > ' ' && *p != '#'; ++p ) {
				;
			}
			if ( (size_t)(p - cp) == len && raw_memcmp(cp, name, len) == 0 ) {
				return cp; /* Found */
			}
		}

		/* Next */
		while ( *cp != '\0' && *cp++ != '\n' ) {
			;
		}
	}

	return 0;
}

