#include "paging.h"
#include "mmap.h"
#include "memory.h"
#include "serial.h"
#include "desc.h"

#define TABLE_ENTRIES_COUNT 512
#define CANON_HOLE_OFFSET 256

int get_ith_bit(const pml4_entry_t * entry, int i);
int get_xd(const pml4_entry_t * entry);
uint64_t get_physical(const pml4_entry_t * entry);
int get_ps(const pml4_entry_t * entry);
int get_a(const pml4_entry_t * entry);
int get_su(const pml4_entry_t * entry);
int get_rw(const pml4_entry_t * entry);
int get_p(const pml4_entry_t * entry);

void set_ith_bit(pml4_entry_t * entry, int val, int i);
void set_xd(pml4_entry_t * entry, int val);
void set_physical(pml4_entry_t * entry, uint64_t val);
void set_ps(pml4_entry_t * entry, int val);
void set_a(pml4_entry_t * entry, int val);
void set_su(pml4_entry_t * entry, int val);
void set_rw(pml4_entry_t * entry, int val);
void set_p(pml4_entry_t * entry, int val);

void default_pml4_entry_setup(struct paging_entry * entry, struct paging_entry * next_table)
{
    set_p(entry, 1);
    set_rw(entry, 1);
    set_su(entry, 0);
    set_xd(entry, 0);
    set_physical(entry, (uint64_t)next_table);    
}

void default_pdpt_entry_setup(struct paging_entry * entry, uint64_t next_table, int ps)
{
    default_pml4_entry_setup(entry, (struct paging_entry *)next_table);
    set_ps(entry, ps);  
}

void init_paging(mb_info_t * mb_info)
{
    pml4_entry_t * pml4_beg = (pml4_entry_t *) find_mem(mb_info, 2 * PAGE_SIZE, PAGE_SIZE);
    pdpt_entry_t * pdpt_beg = (pdpt_entry_t *) ((uint64_t) pml4_beg + PAGE_SIZE);

    // map all pml4 entries to the only pdpt (where else?..)
    // for (size_t i = 0; i < TABLE_ENTRIES_COUNT; ++i)
    // {
    //     pml4_entry_t * curr_entry = (pml4_entry_t *) ((uint64_t) pml4_beg + i);
    //     default_pml4_entry_setup(curr_entry, pdpt_beg);
    // }

    // first 4GB to start
    default_pml4_entry_setup(pml4_beg, pdpt_beg);

    // first 510GB to start (assuming we have no more than 510GB of phys memory)
    pml4_entry_t * after_hole = (pml4_entry_t *) ((uint64_t) pml4_beg + CANON_HOLE_OFFSET);
    default_pml4_entry_setup(after_hole, pdpt_beg);
    
    // last 2GB to start
    pml4_entry_t * last = (pml4_entry_t *) ((uint64_t) pml4_beg + TABLE_ENTRIES_COUNT - 1);
    default_pml4_entry_setup(last, pdpt_beg);

    // map first 510 pdpt entries to first 510GB
    for (uint64_t i = 0; i < TABLE_ENTRIES_COUNT - 2; ++i)
    {
        pdpt_entry_t * curr_entry = (pdpt_entry_t *) ((uint64_t) pdpt_beg + i);
        default_pdpt_entry_setup(curr_entry, i, 1);
    }
    default_pdpt_entry_setup((pdpt_entry_t *) ((uint64_t) pdpt_beg + TABLE_ENTRIES_COUNT - 1), 0, 1);
    default_pdpt_entry_setup((pdpt_entry_t *) ((uint64_t) pdpt_beg + TABLE_ENTRIES_COUNT    ), 1, 1);

    write_to_serial("cr3 write begin\n");
    write_cr3(pml4_beg);
    write_to_serial("cr3 write end\n");
}


int get_ith_bit(const pml4_entry_t * entry, int i)
{
    return (entry->data >> i) & 1;
}

int get_xd(const pml4_entry_t * entry)
{
    return get_ith_bit(entry, 63);
}

uint64_t get_physical(const pml4_entry_t * entry)
{
    return (entry->data >> 12) & 0xffffffffff;
}

int get_ps(const pml4_entry_t * entry)
{
    return get_ith_bit(entry, 7);
}

int get_a(const pml4_entry_t * entry)
{
    return get_ith_bit(entry, 5);
}

int get_su(const pml4_entry_t * entry)
{
    return get_ith_bit(entry, 2);
}

int get_rw(const pml4_entry_t * entry)
{
    return get_ith_bit(entry, 1);
}

int get_p(const pml4_entry_t * entry)
{
    return get_ith_bit(entry, 0);
}



void set_ith_bit(pml4_entry_t * entry, int val, int i)
{ 
    entry->data ^= (-val ^ entry->data) & (1 << i);
}

void set_xd(pml4_entry_t * entry, int val)
{ 
    set_ith_bit(entry, val, 63);
}

void set_physical(pml4_entry_t * entry, uint64_t val)
{
    entry->data ^= (((val & 0xffffffffff) << 12) ^ entry->data);
}

void set_ps(pml4_entry_t * entry, int val)
{
    set_ith_bit(entry, val, 7);
}

void set_a(pml4_entry_t * entry, int val)
{
    set_ith_bit(entry, val, 5);
}

void set_su(pml4_entry_t * entry, int val)
{
    set_ith_bit(entry, val, 2);
}

void set_rw(pml4_entry_t * entry, int val)
{
    set_ith_bit(entry, val, 1);
}

void set_p(pml4_entry_t * entry, int val)
{
    set_ith_bit(entry, val, 0);
}
