// what's the minimum amount of memory that we have available?
// ^ answer this question. Allocate the amount. And then work backwards from there.

// https://blog.randy.gg/memory-management/
// Implementation guide from https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/ 

typedef struct TH_Arena TH_Arena;
struct TH_Arena
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

function void TH_ArenaInit(TH_Arena* arena, void* backing_buffer, size_t backing_buffer_length)
{
	MemoryZeroStruct(arena);
	arena->buffer = (U8*)backing_buffer;
	arena->buffer_length = backing_buffer_length;
}

function void* TH_ArenaPush(TH_Arena* arena, size_t size)
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

function void TH_ArenaClear(TH_Arena* arena)
{
	MemoryZero(arena->buffer, arena->buffer_length);
	arena->offset = 0;
}

// If I ever need resizing - https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/