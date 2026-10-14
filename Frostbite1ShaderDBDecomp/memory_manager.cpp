#include "memory_manager.h"
//#include "multi_core.h"
#include <windows.h>
#include <synchapi.h>

mm_page* g_structs_page = nullptr;
mm_page* g_instances_page = nullptr;
mm_manager memory_manager; 

u32 get_free_space(mm_pool* pool, u32 size);
u32 get_start_pos(mm_pool* pool);
void free_page(u32 pageid);
void free_allocated(void* ptr);
void free_allocated_no_reset(void* ptr);

CRITICAL_SECTION write_mutex; 

mm_pool* create_pool(u32 size)
{
	mm_pool* pool = NULL; 
	mm_manager* manager = get_memory_manager();
	char* poolpos = (char*)malloc((size * sizeof(char)) + sizeof(mm_pool));
	if (size % MEMORY_PAGE_SIZE != 0)
	{
		return pool; 
	}
	pool = (mm_pool*)poolpos; 
	pool->id = manager->pools.size() + 1; 
	pool->start = (char*)(poolpos + sizeof(mm_pool));
	memset(pool->start, 0, size);
	pool->end = pool->start + size * sizeof(char);
	pool->size = size * sizeof(char);
	pool->used = 0;
	new (&pool->pages)std::vector<mm_page*>;
	pool->pages.reserve(12);
	manager->pools.push_back(pool);
	return pool; 
}

void destroy_pool(mm_pool* pool)
{
	mm_manager* manager = get_memory_manager();
	u32 n_pools = manager->pools.size();
	for (int i = 0; i < n_pools; i++)
	{
		if (manager->pools.at(i)->id == pool->id)
		{
			for (int j = 0; j < pool->pages.size(); j++)
			{
				pool->pages.at(j)->fragments.clear();
				pool->pages.at(j)->fragments.shrink_to_fit();
			}
			pool->pages.clear();
			pool->pages.shrink_to_fit(); 
			free((pool->start-sizeof(mm_pool)));
			manager->pools.erase(manager->pools.begin() + i);
			manager->pools.shrink_to_fit(); 
			return;
		}
	}
	return;
}

u32 get_last_pool_id()
{
	mm_manager* manager = get_memory_manager();
	return (manager->pools.size() - 1);
}

mm_pool* get_memory_pool(u32 id)
{
	mm_manager* manager = get_memory_manager();
	return manager->pools.at(id);
}

mm_page* get_memory_page(u32 id)
{
	for (int i = 0; i < memory_manager.pools.size(); i++)
	{
		for (int j = 0; j < memory_manager.pools.at(i)->pages.size(); j++)
		{
			if (memory_manager.pools.at(i)->pages.at(j)->id == id)
			{
				return memory_manager.pools.at(i)->pages.at(j);
			}
		}
	}
	return NULL;
}

void init_memory_manager()
{
	InitializeCriticalSection(&write_mutex);
	memory_manager.create_pool = create_pool;
	memory_manager.get_last_pool_id = get_last_pool_id;
	memory_manager.get_memory_pool = get_memory_pool;
	memory_manager.reserve_pool = reserve_pool;
	memory_manager.get_memory_page = get_memory_page;
	memory_manager.allocate = allocate; 
	memory_manager.pallocate = pallocate;
	memory_manager.free_page = free_page; 
	memory_manager.free_allocated = free_allocated; 
	memory_manager.free_allocated_no_reset = free_allocated_no_reset;
	memory_manager.destroy_pool = destroy_pool; 
	return; 
}

mm_manager* get_memory_manager()
{
	return &memory_manager;
}

// reserves space in memory pool. will add more complex checks later 


mm_page* mm_pool_add_page(mm_pool* pool)
{
	mm_page* page = (mm_page*)((char*)(pool->start + pool->used));
	pool->used += sizeof(mm_page);
	return page; 
}


mm_page* reserve_pool(mm_pool* pool, u32 id, u32 size)
{
	mm_page* page = NULL;
	u32 free_space;
	if ((size / MEMORY_PAGE_SIZE < 1) || (size > (pool->end - pool->start)))
	{
		return page;
	}
	if (pool->pages.size() == 0)
	{
		page = mm_pool_add_page(pool);
		page->id = id;
		page->used = 0; 
		page->chunk.start = pool->start + pool->used;
		page->chunk.size = size;
		page->chunk.is_free = false;
		memset(page->chunk.start, 0, size);
		new (&page->fragments)std::vector<mm_chunk*>;
		pool->used += size; 
		pool->pages.push_back(page);
		return page;
	}
	else
	{
		free_space = get_free_space(pool, size);
		if (free_space < size) { return page; }
		else
		{
			page = mm_pool_add_page(pool);
			page->id = id;
			page->used = 0; 
			page->chunk.start = pool->start + pool->used;
			page->chunk.size = size;
			page->chunk.is_free = false;
			memset(page->chunk.start, 0, size);
			new (&page->fragments)std::vector<mm_chunk*>;
			pool->used += size; 
			pool->pages.push_back(page);
			return page;
		}
	}
}


u32 get_start_pos(mm_pool* pool)
{
	u32 n_pages = pool->pages.size();
	u32 start = 0;
	for (int i = 0; i < n_pages; i++)
	{
		start += pool->pages.at(i)->chunk.size;
	}
	return start;
}


u32 get_free_space(mm_pool* pool, u32 size)
{
	u32 n_pages = pool->pages.size();
	u32 total_space = pool->end - pool->start;
	u32 used_space = 0;
	for (int i = 0; i < n_pages; i++)
	{
		used_space += pool->pages.at(i)->chunk.size;
	}
	return (total_space - used_space);
}


void* allocate_aligned(u32 pageid, u32 size, i32 alignment) // allocates size bytes in page. checks for bounds
{
	if (size == 0) return NULL;
	mm_page* page = get_memory_page(pageid);
	i32 notaligned = size % alignment; 
	u32 align = 0;
	u32 align_dst = 0; 
	i32 dst_notaligned = (u32)(((char*)page->chunk.start) + page->used) % alignment; 
	align_dst = abs(dst_notaligned - alignment);
	if (notaligned)
	{
		align = abs(notaligned - alignment); 
	}
	u32 enough = size + page->used + align + sizeof(mm_chunk);
	EnterCriticalSection(&write_mutex);
	if (enough < page->chunk.size)
	{
		mm_chunk* cnk = (mm_chunk*)(((char*)page->chunk.start) + page->used + align_dst);
		cnk->is_free = false;
		cnk->size = size + alignment;
		cnk->start = (char*)cnk + sizeof(mm_chunk);
		page->cur = (char*)cnk->start + size;
		page->used += size + sizeof(mm_chunk) + alignment;
		page->fragments.push_back(cnk);
		LeaveCriticalSection(&write_mutex);
		return cnk->start;
	}
	else // when space is not available check fragments for free space. slow.
	{
		u32 max = page->fragments.size();
		for (int i = 0; i < max; i++)
		{
			mm_chunk* frag = page->fragments.at(i);
			if ((frag->is_free) && (frag->size >= size))
			{
				frag->is_free = false;
				LeaveCriticalSection(&write_mutex);
				return frag->start;
			}
		}
		LeaveCriticalSection(&write_mutex);
		return nullptr;
	}
}

void* allocate(u32 pageid, u32 size) // allocates size bytes in page. checks for bounds
{
	mm_page* page = get_memory_page(pageid);
	if (size == 0) return NULL;
	u32 enough = size  + page->used + sizeof(mm_chunk);
	EnterCriticalSection(&write_mutex);
	if (enough < page->chunk.size)
	{
		mm_chunk* cnk =  (mm_chunk*)(((char*)page->chunk.start) + page->used);
		cnk->is_free = false;
		cnk->size = size;
		cnk->start = (char*)cnk + sizeof(mm_chunk);
		page->cur = (char*)cnk->start + size; 
		page->used += size + sizeof(mm_chunk);
		page->fragments.push_back(cnk);
		LeaveCriticalSection(&write_mutex);
		return cnk->start; 
	}
	else // when space is not available check fragments for free space. slow.
	{
		u32 max = page->fragments.size();
		for (int i = 0; i < max; i++)
		{
			mm_chunk* frag = page->fragments.at(i);
			if ((frag->is_free) && (frag->size >= size))
			{
				frag->is_free = false; 
				LeaveCriticalSection(&write_mutex);
				return frag->start;
			}
		}
		LeaveCriticalSection(&write_mutex);
		return nullptr;
	}
}

void* pallocate(mm_page* page, u32 size) // allocate in page directly
{
	u32 enough = size + page->used + sizeof(mm_chunk);
	EnterCriticalSection(&write_mutex);
	if (enough < page->chunk.size)
	{
		mm_chunk* cnk = (mm_chunk*)(((char*)page->chunk.start) + page->used);
		cnk->is_free = false;
		cnk->size = size;
		cnk->start = (char*)cnk + sizeof(mm_chunk);
		page->cur = (char*)cnk->start + size;
		page->used += size + sizeof(mm_chunk);
		page->fragments.push_back(cnk);
		LeaveCriticalSection(&write_mutex);
		return cnk->start;
	}
	else // when space is not available check fragments for free space. slow.
	{
		u32 max = page->fragments.size();
		for (int i = 0; i < max; i++)
		{
			mm_chunk* frag = page->fragments.at(i);
			if ((frag->is_free) && (frag->size >= size))
			{
				frag->is_free = false;
				LeaveCriticalSection(&write_mutex);
				return frag->start;
			}
		}
		LeaveCriticalSection(&write_mutex);
		return nullptr;
	}
}

void free_page(u32 pageid)
{
	mm_page* page = get_memory_page(pageid);
	page->chunk.is_free = true; 
	page->used = 0; 
	page->fragments.clear();
	memset(page->chunk.start, 0, page->chunk.size);
}

void free_allocated(void* ptr)
{
	EnterCriticalSection(&write_mutex);
	mm_chunk* chunk = (mm_chunk*)(((char*)(ptr))-sizeof(mm_chunk));
	chunk->is_free = true;
	memset(ptr, 0, chunk->size);
	LeaveCriticalSection(&write_mutex);
}

void free_allocated_no_reset(void* ptr)
{
	EnterCriticalSection(&write_mutex);
	mm_chunk* chunk = (mm_chunk*)(((char*)(ptr)) - sizeof(mm_chunk));
	chunk->is_free = true;
	LeaveCriticalSection(&write_mutex);
}


int write_to_page(mm_page* page, void* src, u32 bytes)
{
	if ((char*)page->cur + bytes <= (char*)page->chunk.start + page->used)
	{
	//	std::cout << "WRITING TO PAGE AT ADDRESS: " << page->cur << " TOTAL BYTES: " << bytes << std::endl;
		memcpy(page->cur, src, bytes);
		return 0;
	}
	else
	{
	//	std::cout << "CANNOT WRITE. BYTES EXCEED ALLOCATED SPACE " << (u32)((u32)page->cur + bytes) << " " << (u32)((u32)page->chunk.start + page->used) << std::endl;
		return -1;
	}
}