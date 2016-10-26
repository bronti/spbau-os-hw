#pragma once

#define MMAP_FLAG (1 << 6)

#include "multiboot_info.h"


int handle_mmap(mb_info_t * mb_info);