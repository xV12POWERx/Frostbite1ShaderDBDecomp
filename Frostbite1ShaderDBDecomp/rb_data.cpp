/*
 * Copyright (c) 2019 xieqing. https://github.com/xieqing
 * May be freely redistributed, but copyright notice must be retained.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rb_data.h"
#include "memory_manager.h"

mydata_mm128*makedata(__m128i key)
{
	mm_manager* mm_mngr = get_memory_manager();
	mydata_mm128*p;

	p = (mydata_mm128*)mm_mngr->allocate(MEMPAGE,sizeof(mydata_mm128));
	if (p != NULL)
		p->key = key;

	return p;
}

int compare_func(const void *d1, const void *d2)
{
	mydata_mm128*p1, *p2;
	
	assert(d1 != NULL);
	assert(d2 != NULL);
	p1 = (mydata_mm128*) d1;
	p2 = (mydata_mm128*) d2;
	__m128i a, b; 
	a = _mm_loadu_epi8((void*)&p1->key);
	b = _mm_loadu_epi8((void*)&p2->key);
//	__m128i vcmp = _mm_cmpeq_epi8(p1->key, p2->key);     
	__m128i vcmp = _mm_cmpeq_epi8(a, b);
	uint16_t vmask = _mm_movemask_epi8(vcmp);              
	if (vmask == 0xffff)
	{
		return 0;
	}
//	if (p1->key == p2->key)
//		return 0;
	else
	{
		__m128i vcmpgt = _mm_cmpgt_epi8(a, b);
		__m128i vcmplt = _mm_cmplt_epi8(a, b);
		if (_mm_movemask_epi8(vcmpgt) > _mm_movemask_epi8(vcmplt))
		//if (p1->key > p2->key)
			return 1;
		else
			return -1;
	}

}

void destroy_func(void *d)
{
	mm_manager* mm_mngr = get_memory_manager();
	mydata_mm128*p;	
	assert(d != NULL);	
	p = (mydata_mm128*) d;
	mm_mngr->free_allocated(p);
}

void print_func(void *d)
{
	mydata_mm128*p;
	
	assert(d != NULL);
	
	p = (mydata_mm128*) d;
	char guid[36] = { 0 };
	u8* pGuid = (u8*)&p->key; 
	for (int i = 0; i < 16; i++)
	{
		sprintf(guid + (i * 2), "%02X", pGuid[i]); 
	}
	printf("%s\n", guid);
}

void print_char_func(void *d)
{
	mydata_mm128*p;
	
	assert(d != NULL);
	
	p = (mydata_mm128*) d;
//	printf("%c", p->key & 127);
}