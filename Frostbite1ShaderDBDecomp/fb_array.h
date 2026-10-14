#pragma once
#include "typedefs.h"
#include "memory_manager.h"
//#include "class_instance.h"

typedef struct fb_array; 

int fb_array_init(fb_array* arr, u32 pageid, u32 size, u32 elemsize);

void fb_array_clear(fb_array* arr);
u32* fb_array_find_u32(fb_array* arr, u32 val);
u32 fb_array_get_u32(fb_array* arr, u32 i);
void fb_array_qsort_u32(fb_array* arr);
i32 fb_array_available(fb_array arr);
i32 fb_array_capacity(fb_array arr);
i32 fb_array_seek(fb_array* arr, void* targetValueAddress); 
i32 fb_array_remove_at(fb_array* arr, u32 i);

u32 fb_array_size(fb_array arr);
u32 fb_array_size(fb_array* arr);
void fb_array_add(fb_array* arr, u32 elem);
void fb_array_add(fb_array* arr, char* elem);
void fb_array_add(fb_array* arr, float elem);
i32 fb_array_add(fb_array* arr, void* item);
//void* fb_array_add(fb_array* arr, struct_descriptor* descr);
//void fb_array_add(fb_array* arr, class_instance* elem);
void* fb_array_get(fb_array* arr, u32 i);



struct fb_array
{
	void* dst;
	u32 used;
//	u32 size;
	u32 available;
	u32 elemsize; 
};
