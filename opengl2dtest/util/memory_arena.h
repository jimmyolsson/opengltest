#pragma once
struct memory_arena
{
	char* base = nullptr;
	unsigned long long total_size = 0;
	unsigned long long used = 0;
};

void memory_arena_init(memory_arena* arena, unsigned long long size);
void* memory_arena_get(memory_arena* arena, unsigned long long size);
void memory_arena_dealloc(memory_arena* arena);
