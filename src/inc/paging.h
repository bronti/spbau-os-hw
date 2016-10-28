#pragma once

#include "multiboot_info.h"

struct paging_entry
{
    uint64_t data;
} __attribute__((packed));
typedef struct paging_entry pml4_entry_t;
typedef struct paging_entry pdpt_entry_t;
typedef struct paging_entry pdt_entry_t;


void init_paging(mb_info_t * mb_info);
