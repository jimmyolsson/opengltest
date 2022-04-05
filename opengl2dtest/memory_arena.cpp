#include "memory_arena.h"
#include <stdlib.h>
#include <assert.h>

void memory_arena_init(memory_arena* arena, unsigned long long size)
{
	arena->base = (char*)malloc(size);
	arena->total_size = size;
	arena->used = 0;
}

void* memory_arena_get(memory_arena* arena, unsigned long long size)
{
	assert(arena->used + size <= arena->total_size);

	arena->used += size;
	return (char*)arena->base + arena->used - size;
}

void memory_arena_dealloc(memory_arena* arena)
{
	free(arena->base);
}
