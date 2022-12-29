#include "memory_arena.h"
#include <stdlib.h>
#include <assert.h>

void memory_arena_init(memory_arena* self, unsigned long long total_size, unsigned long long piece_size)
{
	self->base = (char*)malloc(total_size);
	self->total_size = total_size;
	self->piece_size = piece_size;
	self->used = 0;
}

void memory_arena_init(memory_arena* self, unsigned long long total_size, unsigned long long piece_size, short alignment)
{
	self->base = (char*)_aligned_malloc(total_size, alignment);
	self->total_size = total_size;
	self->piece_size = piece_size;
	self->used = 0;
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
