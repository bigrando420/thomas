// what's the minimum amount of memory that we have available?
// ^ answer this question. Allocate the amount. And then work backwards from there.

// https://blog.randy.gg/memory-management/
// Implementation guide from https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/ 

typedef struct M_Arena M_Arena;
struct M_Arena
{
	U8* buffer;
	size_t buffer_length;
	size_t offset;
};

function B8 IsPowerOfTwo(uintptr_t x)
{
	return (x & (x - 1)) == 0;
}

function uintptr_t AlignPointerForward(uintptr_t ptr, size_t align)
{
	uintptr_t p, a, modulo;
	Assert(IsPowerOfTwo(align));
	p = ptr;
	a = (uintptr_t)align;
	modulo = p & (a - 1);
	if (modulo != 0)
	{
		p += a - modulo;
	}
	return p;
}

function void M_ArenaInit(M_Arena* arena, void* backing_buffer, size_t backing_buffer_length)
{
	MemoryZeroStruct(arena);
	arena->buffer = (U8*)backing_buffer;
	arena->buffer_length = backing_buffer_length;
}

function void* M_ArenaAlloc(M_Arena* arena, size_t size)
{
	size_t align = 2 * sizeof(void*);
	uintptr_t cur_ptr = (uintptr_t)arena->buffer + (uintptr_t)arena->offset;
	uintptr_t offset = AlignPointerForward(cur_ptr, align);
	offset -= (uintptr_t)arena->buffer;

	Assert(offset + size <= arena->buffer_length);
	void* ptr = &arena->buffer[offset];
	arena->offset = offset + size;
	MemorySet(ptr, 0, size);
	return ptr;
}

function void M_ArenaClear(M_Arena* arena)
{
	MemoryZero(arena->buffer, arena->buffer_length);
	arena->offset = 0;
}

// If I ever need resizing - https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/


// POOL
/*

I'm pretty sure this is whole reason I wanted to try out C++, was because I found myself rewriting the freeable array a lot of times, thinking a template would fix it.

This memory arena pool idea seems rock solid though, and a solid way of creating a generic freeable structure.

*/

// https://www.gingerbill.org/article/2019/02/16/memory-allocation-strategies-004/

typedef struct M_PoolFreeNode M_PoolFreeNode;
struct M_PoolFreeNode
{
	M_PoolFreeNode* next;
};

typedef struct M_Pool M_Pool;
struct M_Pool
{
	U8* buffer;
	size_t buffer_length;
	size_t chunk_size;
	M_PoolFreeNode* first;
};

function void M_PoolInit(M_Pool* pool, void* backing_buffer, size_t backing_buffer_length, size_t chunk_size, size_t chunk_alignment)
{
	MemoryZeroStruct(pool);
	
	// Align buffer_length and chunk_size to chunk_alignment
	uintptr_t initial_start = (uintptr_t)backing_buffer;
	uintptr_t start = AlignPointerForward(initial_start, chunk_alignment);
	backing_buffer_length -= (size_t)(start - initial_start);
	chunk_size = AlignPointerForward(chunk_size, chunk_alignment);
	
	Assert(chunk_size >= sizeof(M_PoolFreeNode) && "The node has to fit within the chunk size");
	Assert(backing_buffer_length >= chunk_size && "Backing buffer is too small to fit even a single chunk");
	
	pool->buffer = (U8*)backing_buffer;
	pool->buffer_length = backing_buffer_length;
	pool->chunk_size = chunk_size;
}

function void* M_PoolAlloc(M_Pool* pool)
{
	M_PoolFreeNode* node = pool->first;
	Assert(node && "No space left in pool.");
	StackPop(pool->first);
	MemoryZero(node, pool->chunk_size);
	return node;
}

function void M_PoolFree(M_Pool* pool, void* ptr)
{
	Assert(ptr && "invalid pointer");
	void* start = pool->buffer;
	void* end = &pool->buffer[pool->buffer_length];
	Assert(ptr >= start && ptr <= end && "Pointer is out of bounds of this pool's buffer");
	M_PoolFreeNode* node = (M_PoolFreeNode*)ptr;
	StackPush(pool->first, node);
}

function void M_PoolFreeAll(M_Pool* pool)
{
	size_t chunk_count = pool->buffer_length / pool->chunk_size;
	for (int i = 0; i < chunk_count; i++) {
		void* ptr = &pool->buffer[i * pool->chunk_size];
		M_PoolFree(pool, ptr);
	}
}