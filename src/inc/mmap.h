#pragma once

#define MMAP_FLAG (1 << 6)

#include "multiboot_info.h"


int handle_mmap(mb_info_t * mb_info);
uint64_t find_mem(mb_info_t * mb_info, uint64_t size, uint64_t align);
// void reserve_memory_block(mb_info_t * mb_info, uint32_t block_start, uint32_t block_size);