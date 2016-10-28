#include "buddy_allocator.h"
#include "memory.h"
#include "serial.h"
#include "mmap.h"
#include <stdint.h>


// numeration from 1 (like in heap)
struct block * descriptors; //[BLOCKS_COUNT];
struct block * lists_heads; //[BLOCK_EXP + 1];

uint64_t to_phys(uint64_t addr)
{
    return addr - CANONICAL_HOLE_OFFSET;
}

uint64_t to_logic(uint64_t addr)
{
    return addr + CANONICAL_HOLE_OFFSET;
}

uint64_t block_size_from_exp(int exp)
{
    return PAGE_SIZE * (1ull << exp);
}

uint64_t block_ind(uint64_t phys_start, int exp)
{
    uint64_t num_in_line = phys_start / block_size_from_exp(exp);
    uint64_t bigger_blocks = 1ull << (BLOCK_EXP - exp);
    return bigger_blocks + num_in_line;
}

uint64_t block_phys(uint64_t ind, int exp)
{
    uint64_t bigger_blocks = 1ull << (BLOCK_EXP - exp);
    uint64_t blocks_before = ind - bigger_blocks;
    return block_size_from_exp(exp) * blocks_before;
}

uint64_t block_ind_by_descr(struct block * descr)
{
    return ((uint64_t)descr - (uint64_t)descriptors) / sizeof(struct block);
}

int is_availible(uint64_t ind)
{
    struct block * descr = &(descriptors[ind]);
    return (uint64_t)descr->next != (uint64_t)descr;
}

uint64_t get_buddy_ind(uint64_t ind) 
{    
    return ind ^ 1ull;
}

uint64_t get_parent_ind(uint64_t ind) 
{    
    return ind / 2;
}

uint64_t get_left_child_ind(uint64_t ind) 
{    
    return 2 * ind;
}

uint64_t get_right_child_ind(uint64_t ind) 
{    
    return 2 * ind + 1;
}

int no_free_of_exp(int exp)
{
    return lists_heads[exp].next == &lists_heads[exp];
}

void add_block_to_free_list(uint64_t ind, int exp)
{
    struct block * descr = &(descriptors[ind]);

    descr->next = lists_heads[exp].next;
    descr->prev = &lists_heads[exp];
    lists_heads[exp].next = descr;
    descr->next->prev = descr;
}

void remove_block_from_free_list(uint64_t ind)
{
    struct block * descr = &(descriptors[ind]);

    descr->prev->next = descr->next;
    descr->next->prev = descr->prev;
    descr->next = descr;
    descr->prev = descr;
}

void free_block_by_ind(uint64_t ind, int exp)
{
    uint64_t buddy_ind = get_buddy_ind(ind);
    if (exp < BLOCK_EXP && is_availible(buddy_ind))
    {
        remove_block_from_free_list(buddy_ind);
        free_block_by_ind(get_parent_ind(ind), exp + 1);
    }
    else 
    {
        add_block_to_free_list(ind, exp);
    }
}

uint64_t get_free_block(int exp)
{
    if (exp == BLOCK_EXP && no_free_of_exp(exp))
    {
        // cannot allocate memory
        return 0;
    }
    if (no_free_of_exp(exp))
    {
        uint64_t parent = get_free_block(exp + 1);
        if (parent == 0) return 0;
        free_block_by_ind(get_right_child_ind(parent), exp);
        // remove_block_from_free_list(get_left_child_ind(parent));
        return get_left_child_ind(parent);
    }
    struct block * descr = lists_heads[exp].next;
    uint64_t ind = block_ind_by_descr(descr);
    remove_block_from_free_list(ind);
    return ind;
}

void init_before(int max_exp, uint64_t entry_end, uint64_t start)
{
    int exp;
    uint64_t block_start;
    for (exp = max_exp; exp >= 0; --exp)
    {
        uint64_t block_size = block_size_from_exp(exp);
        block_start = entry_end - block_size;
        if (block_start < start) continue;
        free_block_by_ind(block_ind(block_start, exp), exp);
        break;
    }
    if (exp >= 1 && block_start > start)
    {
        init_before(exp - 1, block_start, start);
    }
}

void init_after(int max_exp, uint64_t start, uint64_t entry_end)
{
    for (int exp = max_exp; exp >= 0; --exp)
    {
        uint64_t block_size = block_size_from_exp(exp);
        if (start + block_size > entry_end) continue;
        free_block_by_ind(block_ind(start, exp), exp);
        start += block_size;
    }
}

int init_buddy_allocator(mb_info_t * mb_info)
{    
    uint64_t descriptors_mem = sizeof(struct block) * BLOCKS_COUNT;
    uint64_t lists_heads_mem = sizeof(struct block) * (BLOCK_EXP + 1);

    descriptors = (struct block *) find_mem(mb_info, descriptors_mem + lists_heads_mem, 1);
    if (descriptors == 0) return 0;

    lists_heads = (struct block *) (descriptors + BLOCKS_COUNT);

    uint64_t mmap_begin = (uint64_t) mb_info->mmap_addr;
    uint64_t mmap_size  = (uint64_t) mb_info->mmap_length;
    uint64_t mmap_end   = mmap_begin + mmap_size;
    uint64_t mmap_entry_size = ((mb_memory_map_t *) mmap_begin)->size +
                               sizeof(((mb_memory_map_t *) mmap_begin)->size);

    for (int i = 0; i <= BLOCK_EXP; ++ i)
    {
        lists_heads[i].next = &(lists_heads[i]);
        lists_heads[i].prev = &(lists_heads[i]);
    }
    for (uint64_t i = 0; i < BLOCKS_COUNT; ++ i)
    {
        descriptors[i].next = &(descriptors[i]);
        descriptors[i].prev = &(descriptors[i]);
    }

    for(mb_memory_map_t * entry = (mb_memory_map_t *) mmap_begin; 
        (uint64_t) entry < mmap_end; 
        entry = (mb_memory_map_t *)((uint64_t) entry + mmap_entry_size))
    {
        if (entry->type == MB_MEMORY_RESERVED) continue;
        if (entry->len < PAGE_SIZE) continue;

        uint64_t entry_end = entry->addr + entry->len;
        uint64_t start = entry->addr;

        int32_t exp;
        uint64_t gap;
        uint64_t block_size;
        for (exp = BLOCK_EXP; exp >= 0; --exp)
        {
            block_size = block_size_from_exp(exp);
            gap = start % block_size == 0 ? 0 : (block_size - start % block_size);
            if (start + gap + block_size > entry_end) continue;
            // start += gap;
            free_block_by_ind(block_ind(start + gap, exp), exp);
            // start += block_size;
            break;
        }
        if (exp >= 1 && gap > 0)
        {
            init_before(exp - 1, start + gap, start);
        }
        if (exp >= 1 && start + gap + block_size < entry_end)
        {
            init_after(exp - 1, start + gap + block_size, entry_end);
        }
    }
    return 1;
}

void free_block(uint64_t logic_start, int exp)
{
    free_block_by_ind(block_ind(to_phys(logic_start), exp), exp);
}

uint64_t alloc_block(int exp)
{
    uint64_t ind = get_free_block(exp);
    if (ind == 0) return 0;
    uint64_t phys = block_phys(ind, exp);
    return to_logic(phys);
}

int first_non_zero(uint64_t val)
{
    int i = 0;
    while(val > 1)
    {
        ++i;
        val >>= 1;
    }
    return i;
}

int free_blocks(uint64_t logic_start, uint64_t size)
{
    if (size % PAGE_SIZE > 0)
    {
        return 0;
    }
    size = size / PAGE_SIZE;
    int exp = first_non_zero(size);
    if (size == (1ull << exp))
    {
        free_block(logic_start, exp);
        return 1;
    }
    int curr_exp = exp;
    uint64_t curr = block_ind(to_phys(logic_start), exp + 1);
    while (size > 0)
    {
        if (size < (1ull << curr_exp))
        {
            curr = get_left_child_ind(curr);
        }
        else 
        {
            free_block_by_ind(get_left_child_ind(curr), curr_exp);
            size -= (1ull << curr_exp);
            curr = get_right_child_ind(curr);
        }
        --curr_exp;
    }
    return 1;
}

uint64_t alloc_blocks(uint64_t size)
{
    if (size % PAGE_SIZE > 0)
    {
        return 0;
    }
    size = size / PAGE_SIZE;
    int exp = first_non_zero(size);
    if (size == (1ull << exp))
    {
        return alloc_block(exp);
    }
    uint64_t start_ind = get_free_block(exp + 1);
    if (start_ind == 0) return 0;
    int curr_exp = exp;
    uint64_t curr_ind = start_ind;
    while (1)
    {
        if (size == 0)
        {
            free_block_by_ind(curr_ind, curr_exp + 1);
            break;
        }
        else if (size < (1ull << curr_exp))
        {
            free_block_by_ind(get_right_child_ind(curr_ind), curr_exp);
            curr_ind = get_left_child_ind(curr_ind);
        }
        else 
        {
            size -= (1ull << curr_exp);
            curr_ind = get_right_child_ind(curr_ind);
        }
        --curr_exp;
    }
    return to_logic(block_phys(start_ind, exp + 1));
}

void show_buddies()
{
    for (int i = BLOCK_EXP; i >= 0; --i)
    {
        write_to_serial("show free blocks on level ");
        write_num_to_serial((uint64_t)i, '\n');
        struct block * head = &(lists_heads[i]);
        struct block * curr = head->next;
        while ((uint64_t)curr != (uint64_t)head)
        {
            uint64_t ind = block_ind_by_descr(curr);
            write_num_to_serial(block_phys(ind, i), '\n');
            curr = curr->next;
        }
    }
}