#pragma once

#include "multiboot_info.h"

// max block -- 8GB (why??)
#define BLOCK_EXP 21
#define MAX_BLOCK_LEN (1ull << BLOCK_EXP)
// numeration from 1 (like in heap)
#define BLOCKS_COUNT (2ull * (1ull << BLOCK_EXP))

#define CANONICAL_HOLE_OFFSET (~((1ull << 47) - 1ull))
#define PAGE_SIZE 0x1000

struct block
{
    struct block * prev;
    struct block * next;
};

int init_buddy_allocator(mb_info_t * mb_info);
void show_buddies();
uint64_t alloc_blocks(uint64_t size);
int free_blocks(uint64_t logic_start, uint64_t size);
// uint64_t alloc_block(int exp);
// void free_block(uint64_t logic_start, int exp);
