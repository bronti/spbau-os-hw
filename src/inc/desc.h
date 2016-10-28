#ifndef __DESC_H__
#define __DESC_H__

#include <stdint.h>
#include "paging.h"

struct desc_table_ptr {
	uint16_t size;
	uint64_t addr;
} __attribute__((packed));

static inline void read_idtr(struct desc_table_ptr *ptr)
{
	__asm__ ("sidt %0" : "=m"(*ptr));
}

static inline void write_idtr(const struct desc_table_ptr *ptr)
{
	__asm__ ("lidt %0" : : "m"(*ptr));
}

static inline void read_gdtr(struct desc_table_ptr *ptr)
{
	__asm__ ("sgdt %0" : "=m"(*ptr));
}

static inline void write_gdtr(const struct desc_table_ptr *ptr)
{
	__asm__ ("lgdt %0" : : "m"(*ptr));
}

static inline void read_cr3(pml4_entry_t * ptr)
{
    __asm__ ("movq %%cr3, %0" : "=r"(*ptr));
}

static inline void write_cr3(const pml4_entry_t * ptr)
{
    __asm__ ("movq %0, %%cr3" : : "r"(*ptr));
}

#endif /*__DESC_H__*/
