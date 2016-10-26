#pragma once

#define MMAP_FLAG (1 << 6)

#include "multiboot_info.h"


void show_mmap(mb_info_t * mb_info);