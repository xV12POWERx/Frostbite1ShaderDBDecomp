#include "fb_array.h"
//#include "program.h"

#define ARRAY_SPACE_EXTRA 1.4

int cmpasc(const void* a, const void* b)
{
	return(*(int*)a - *(int*)b);
}


u32 fb_array_size(fb_array* arr)
{
	return arr->used; 
}

u32 fb_array_size(fb_array arr)
{
	return arr.used;
}

i32 fb_array_available(fb_array arr)
{
	return arr.available - arr.used;
}

i32 fb_array_capacity(fb_array arr)
{
	return arr.available; 
}

int fb_array_init(fb_array* arr, u32 pageid, u32 size, u32 elemsize)
{
	mm_manager* mm_mngr = get_memory_manager();
	arr->dst = mm_mngr->allocate(pageid, (size * ARRAY_SPACE_EXTRA)* elemsize);
	if (arr->dst)
	{
		arr->available = (size * ARRAY_SPACE_EXTRA);
		arr->used = 0;
		arr->elemsize = elemsize; 
		return OK; 
	}
	else return ERROR; 
}

void fb_array_qsort_u32(fb_array* arr)
{
	qsort(arr->dst, fb_array_size(arr), sizeof(u32), cmpasc);
}

u32* fb_array_find_u32(fb_array* arr, u32 val)
{
	u32 max = fb_array_size(arr);
	u32* src = (u32*)arr->dst; 
	for (u32 i = 0; i < max; i++)
	{
		if (*src == val) return src;
		src++;
	}
	return NULL; 
}

u32 fb_array_get_u32(fb_array* arr, u32 i)
{
	return *((u32*)arr->dst + i);
}


void fb_array_add(fb_array* arr, u32 elem)
{
	i32 size = fb_array_available(*arr);
	if (size)
	{
		u32* dst = (u32*)(arr->dst) + fb_array_size(arr);
		*dst = elem;
		arr->used++;
	}
	return;
}

void fb_array_add(fb_array* arr, char* elem)
{
	i32 size = fb_array_available(*arr);
	if (size > 0)
	{
		char** dst = (char**)(arr->dst) + fb_array_size(arr);
		*dst = elem;
		arr->used++;
	}
	return;
}

void fb_array_add(fb_array* arr, float elem)
{
	i32 size = fb_array_available(*arr);
	if (size > 0)
	{
		float* dst = (float*)(arr->dst) + fb_array_size(arr);
		*dst = elem;
		arr->used++;

	}
	return;
}

/*
void fb_array_add(fb_array* arr, class_instance* elem)
{
	i32 size = fb_array_available(*arr);
	if (size > 0)
	{
		class_instance** dst = (class_instance**)(arr->dst) + fb_array_size(arr);
		*dst = elem;
	//	arr->available--;
		arr->used++;

	}
	return;
}


void* fb_array_add(fb_array* arr, struct_descriptor* descr)
{
	i32 size = fb_array_available(*arr);
	void* res = NULL; 
	if (size > 0)
	{
		res = (char*)(arr->dst) + (fb_array_size(arr) * descr->size);
	//	arr->available--;
		arr->used++;

	}
	return res;
}*/

i32 fb_array_add(fb_array* arr, void* item)
{
	i32 size = fb_array_available(*arr);
	i32 dst_index = -1;
	if (size > 0)
	{
		dst_index = fb_array_size(arr);
		char* dst = (char*)arr->dst + (dst_index * arr->elemsize);
		memcpy(dst, item, arr->elemsize);
		arr->used++;

	}
	return dst_index;
}

// RETURNS THE ADDRESS. NOT THE VALUE!
void* fb_array_get(fb_array* arr, u32 i) 
{
	return (void*)((char*)(arr->dst) + (i * arr->elemsize));
}


// ZERO BASED INDEX 
i32 fb_array_remove_at(fb_array* arr, u32 i)
{
	u32 size = fb_array_size(arr); 
	if (i >= size) return ERROR; 
	else
	{
		if ((i == 0) || (i == (size - 1))) memset((char*)arr->dst + (i * arr->elemsize), 0, arr->elemsize);
		else
		{
			u32 num_elements_forward = size - i - 1;
			memmove((char*)arr->dst + (i * arr->elemsize), (char*)arr->dst + ((i + 1) * arr->elemsize), arr->elemsize * num_elements_forward);
		}
		arr->used--;
		return OK; 
	}
}


i32 fb_array_seek(fb_array* arr, void* targetValueAddress)
{
	for (int i = 0; i < fb_array_size(arr); i++)
	{
		if (memcmp(fb_array_get(arr, i), targetValueAddress, arr->elemsize) == 0) return i;
	}
	return ERROR; 
}


void fb_array_clear(fb_array* arr)
{
	arr->used = 0; 
	return; 
}