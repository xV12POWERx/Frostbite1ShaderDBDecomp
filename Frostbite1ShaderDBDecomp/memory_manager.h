#pragma once
#include "typedefs.h"
#include <vector>


#define MEMORY_PAGE_SIZE 64 // THE MINIMUM SIZE FOR AN ALLOCATION. MEMORY POOLS SHOULD BE A MULTIPLE OF THIS 
#define MEGABYTE 1024000
//#define STRUCTS_PAGE 555
#define INSTANCES_PAGE 556
//#define ASSETS_PAGE 557
#define GUI_PAGE 558
#define SHADERDB_PAGE 559
#define TEMP_TEXTURE_PAGE 560
#define THREAD_POOL_PAGE  561 
#define FRAMEBUFFER_PAGE  562
#define LEVEL_PAGE        563
#define STARTUP_PAGE      564
#define THREADS_WORK_PAGE 565
#define MEMPAGE 666
#define MEMPAGETEMP 333
//#define CONSTANTSPAGE 222

typedef struct  mm_pool; 
typedef struct  mm_page;
typedef struct  mm_chunk;
typedef struct  mm_manager;

extern mm_page* g_structs_page;
extern mm_page* g_instances_page;

mm_pool* create_pool(u32 size);
mm_page* reserve_pool(mm_pool* pool, u32 id, u32 size);
mm_manager* get_memory_manager();
void destroy_pool(mm_pool* pool);
void free_allocated(void* ptr);
void* allocate(u32 pageid, u32 size);
void* pallocate(mm_page* page, u32 size); // allocate in page directly
void* allocate_aligned(u32 pageid, u32 size, i32 alignment);
int write_to_page(mm_page* page, void* src, u32 bytes);
void init_memory_manager();

struct alignas(16) mm_chunk
{
	void* start;
	u32 size;
	bool is_free;
};

struct mm_pool
{
	u16 id;
	u32 size;
	u32 used; 
	char* start;
	char* end; 
	std::vector<mm_page*>pages;
};

struct mm_page
{
	u32 id;
	void* cur;
	u32 used;
	mm_chunk chunk;
	std::vector<mm_chunk*>fragments;
};

struct mm_manager
{
	std::vector<mm_pool*> pools;
	mm_pool*(*create_pool)(u32 size);
	u32(*get_last_pool_id)();
	mm_page*(*reserve_pool)(mm_pool* pool, u32 id, u32 size);
	mm_pool*(*get_memory_pool)(u32 id);
	mm_page*(*get_memory_page)(u32 id);
	void* (*allocate)(u32 pageid, u32 size);
	void* (*pallocate)(mm_page* page, u32 size);
	void (*free_page)(u32 pageid);
	void (*free_allocated)(void* ptr);
	void (*free_allocated_no_reset)(void* ptr);
	void (*destroy_pool)(mm_pool* pool);

};