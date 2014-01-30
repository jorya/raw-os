/*
 * Two Levels Segregate Fit memory allocator (TLSF)
 * Version 2.4.6
 *
 * Written by Miguel Masmano Tello <mimastel@doctor.upv.es>
 *
 * Thanks to Ismael Ripoll for his suggestions and reviews
 *
 * Copyright (C) 2008, 2007, 2006, 2005, 2004
 *
 * This code is released using a dual license strategy: GPL/LGPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of the GNU General Public License Version 2.0
 * Released under the terms of the GNU Lesser General Public License Version 2.1
 *
 */

#ifndef _TLSF_H_
#define _TLSF_H_

RAW_U32 init_memory_pool(RAW_U32, void *);
RAW_U32 get_used_size(void *);
RAW_U32 get_max_size(void *);
void destroy_memory_pool(void *);
RAW_U32 add_new_area(void *, RAW_U32, void *);
void *malloc_ex(RAW_U32, void *);
void free_ex(void *, void *);
void *realloc_ex(void *, RAW_U32, void *);
void *calloc_ex(RAW_U32, RAW_U32, void *);

void *tlsf_malloc(RAW_U32 size);
void tlsf_free(void *ptr);
void *tlsf_realloc(void *ptr, RAW_U32 size);
void *tlsf_calloc(RAW_U32 nelem, RAW_U32 elem_size);

#endif
