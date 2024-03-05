#include "memory_arena.h"
#include <stdlib.h>
#include <assert.h>
#include <mutex>

void memory_arena_init(memory_arena* self, unsigned long long total_size, unsigned long long piece_size)
{
	self->base = (char*)calloc(total_size, 1);
	self->total_size = total_size;
	self->piece_size = piece_size;
}

void memory_arena_init(memory_arena* self, unsigned long long total_size, unsigned long long piece_size, short alignment)
{
#ifdef __EMSCRIPTEN__
	self->base = (char*)aligned_alloc(total_size, alignment);
#else
	self->base = (char*)_aligned_malloc(total_size, alignment);
#endif
	self->total_size = total_size;
	self->piece_size = piece_size;
	self->alignment = alignment;
}

void* memory_arena_get(memory_arena* self)
{
	assert(self->used + self->piece_size <= self->total_size && "Arena is full");

	char* ptr = (char*)self->base + self->used;
	self->used += self->piece_size;

	return ptr;
}

void memory_arena_dealloc(memory_arena* self)
{
	free(self->base);
}
