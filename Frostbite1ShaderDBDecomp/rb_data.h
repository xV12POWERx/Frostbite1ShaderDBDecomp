/*
 * Copyright (c) 2019 xieqing. https://github.com/xieqing
 * May be freely redistributed, but copyright notice must be retained.
 */

#ifndef _RB_DATA_HEADER
#define _RB_DATA_HEADER

#include <immintrin.h>

typedef struct {
	__m128i key;
	void* data; 
} mydata_mm128;

mydata_mm128* makedata(__m128i key);
int compare_func(const void *d1, const void *d2);
void destroy_func(void *d);
void print_func(void *d);
void print_char_func(void *d);

#endif /* _RB_DATA_HEADER */

