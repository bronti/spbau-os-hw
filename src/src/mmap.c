#include "mmap.h"
#include "multiboot_info.h"
#include <stdint.h>
#include "string.h"
#include "serial.h"

#define MMAP_ADDITIONAL_ENTRIES 7

int check_flag(uint32_t flags, uint32_t flag)
{
    return (flags & flag) != 0;
}

void reserve_memory_block(mb_info_t * mb_info, uint32_t block_start, uint32_t block_size) 
{
    uint64_t mmap_begin = (uint64_t) mb_info->mmap_addr;
    uint64_t mmap_size  = (uint64_t) mb_info->mmap_length;
    uint64_t mmap_end   = mmap_begin + mmap_size;
    uint64_t mmap_entry_size = ((mb_memory_map_t *) mmap_begin)->size +
                               sizeof(((mb_memory_map_t *) mmap_begin)->size);

    uint32_t block_end = block_start + block_size;
    
    mb_memory_map_t * entry;
    for(entry = (mb_memory_map_t *) mmap_begin; 
        ((uint64_t) entry < mmap_end) && (block_end >= entry->addr); 
        entry = (mb_memory_map_t *)((uint64_t) entry + mmap_entry_size))
    {
        if (block_start >= entry->addr + entry->len) continue;
        if (entry->type == MB_MEMORY_RESERVED) continue;
        break;
    }
    if ((uint64_t) entry >= mmap_end) return;

    mb_memory_map_t * next_entry = (mb_memory_map_t *)((uint64_t) entry + mmap_entry_size);
    mb_memory_map_t * aft_next_entry = (mb_memory_map_t *)((uint64_t) next_entry + mmap_entry_size);

    if (block_end >= entry->addr + entry->len)
    {
        if (block_start <= entry->addr) 
        {
            entry->type = MB_MEMORY_RESERVED;
            return;
        }
        memmove((void *)next_entry, (void *)entry, mmap_end - (uint64_t) entry);
        next_entry->addr = block_start;
        next_entry->len = entry->addr + entry->len - next_entry->addr;
        next_entry->type = MB_MEMORY_RESERVED;
        entry->len = next_entry->addr - entry->addr;
        mb_info->mmap_length += mmap_entry_size;
        return;
    }
    if (block_start <= entry->addr)
    {
        memmove((void *)next_entry, (void *)entry, mmap_end - (uint64_t) entry);
        next_entry->len = entry->addr + entry->len - block_end;
        next_entry->addr = block_end;
        next_entry->type = MB_MEMORY_AVAILABLE;
        entry->len = next_entry->addr - entry->addr;
        entry->type = MB_MEMORY_RESERVED;   
        mb_info->mmap_length += mmap_entry_size;    
        return;     
    }
    memmove((void *) aft_next_entry, (void *)entry, mmap_end - (uint64_t) entry);
    aft_next_entry->len = entry->addr + entry->len - block_end;
    aft_next_entry->addr = block_end;
    aft_next_entry->type = MB_MEMORY_AVAILABLE;
    next_entry->addr = block_start;
    next_entry->len = aft_next_entry->addr - next_entry->addr;
    next_entry->type = MB_MEMORY_RESERVED;
    entry->len = next_entry->addr - entry->addr;
    mb_info->mmap_length += 2 * mmap_entry_size;
}

int restore_mmap(mb_info_t * mb_info, uint64_t os_begin, uint64_t os_end)
{
    uint64_t mmap_begin = (uint64_t) mb_info->mmap_addr;
    uint64_t mmap_size  = (uint64_t) mb_info->mmap_length;
    uint64_t mmap_end   = mmap_begin + mmap_size;
    uint64_t mmap_entry_size = ((mb_memory_map_t *) mmap_begin)->size +
                               sizeof(((mb_memory_map_t *) mmap_begin)->size);
    uint64_t mmap_max_size = mmap_size + MMAP_ADDITIONAL_ENTRIES * mmap_entry_size;

    mb_memory_map_t * entry;
    for(entry = (mb_memory_map_t *) mmap_begin; 
        (uint64_t) entry < mmap_end; 
        entry = (mb_memory_map_t *)((uint64_t) entry + mmap_entry_size))
    {
        if (entry->type == MB_MEMORY_RESERVED) continue;
        if (entry->len < mmap_max_size) continue;
        if (entry->addr >= ((uint64_t)1) << 32) continue;
        if (os_begin <= entry->addr && entry->addr < os_end) continue;
        if (entry->addr <= os_begin && os_begin < entry->addr + mmap_max_size) continue;
        break;
    }

    if ((uint64_t) entry >= mmap_end) return 0;

    memmove((void *) entry->addr, (void *) mmap_begin, mmap_max_size);
    mb_info->mmap_addr = (uint32_t) entry->addr;

    reserve_memory_block(mb_info, mb_info->mmap_addr, mmap_max_size);

    return 1;
}

uint64_t find_mem(mb_info_t * mb_info, uint64_t size, uint64_t align)
{
    uint64_t mmap_begin = (uint64_t) mb_info->mmap_addr;
    uint64_t mmap_size  = (uint64_t) mb_info->mmap_length;
    uint64_t mmap_end   = mmap_begin + mmap_size;
    uint64_t mmap_entry_size = ((mb_memory_map_t *) mmap_begin)->size +
                               sizeof(((mb_memory_map_t *) mmap_begin)->size);

    mb_memory_map_t * entry;
    for(entry = (mb_memory_map_t *) mmap_begin; 
        (uint64_t) entry < mmap_end; 
        entry = (mb_memory_map_t *)((uint64_t) entry + mmap_entry_size))
    {
        if (entry->type == MB_MEMORY_RESERVED) continue;
        uint64_t gap = entry->addr % align == 0 ? 0 : (align - entry->addr % align);
        uint64_t start = entry->addr + gap;
        if (entry->addr + entry->len < start + size) continue;

        reserve_memory_block(mb_info, start, size);
        return start;
    }
    return 0;
}

void show_mmap(mb_info_t * mb_info) 
{
    if (check_flag(mb_info->flags, MMAP_FLAG))
    {
        // write_to_serial("\naddr: ");
        // write_num_to_serial((uint64_t) mb_info->mmap_addr, ' ');
        // write_to_serial("\nlength: ");
        // write_num_to_serial((uint64_t) mb_info->mmap_length, '\n');

        for(mb_memory_map_t * mmap = (mb_memory_map_t *) ((uint64_t) mb_info->mmap_addr);
            (uint64_t) mmap < mb_info->mmap_addr + mb_info->mmap_length; 
            mmap = (mb_memory_map_t *)((uint64_t) mmap + mmap->size + sizeof(mmap->size)))
        {
                write_to_serial("\nsize: ");
                write_num_to_serial((uint64_t) mmap->size, '\n');           
                write_to_serial("addr: ");
                write_num_to_serial(mmap->addr, '\n');                
                write_to_serial("len: ");
                write_num_to_serial(mmap->len, '\n');          
                write_to_serial("type: ");
                write_num_to_serial((uint64_t) mmap->type, '\n');
        }        
        write_to_serial("\n");
    }
    else
    {
        write_to_serial("mmap_* not availible ):\n Shutting down\n");
    }
}

int handle_mmap(mb_info_t * mb_info)
{
    if (0)
    {
        mb_info = mb_info + 1;
    }
    extern char text_phys_begin;
    // extern char data_phys_end;
    extern char bss_phys_end;
    uint64_t os_begin = (uint64_t) &text_phys_begin;
    uint64_t os_end = (uint64_t) &bss_phys_end;
    uint32_t os_len = os_end - os_begin;

    // write_to_serial("\nInitial memory map:\n");
    // show_mmap(mb_info);
    if (! restore_mmap(mb_info, os_begin, os_end))
    {
        write_to_serial("\nRestoring mmap unsuccessfull ):\n");
        return 0;
    }
    // write_to_serial("\nMemory map (after restoring):\n");
    // show_mmap(mb_info);

    // write_to_serial("os start: ");
    // write_num_to_serial(os_begin, '\n'); 
    // write_to_serial("os end: ");
    // write_num_to_serial(os_end, '\n'); 
    // write_to_serial("os len: ");
    // write_num_to_serial(os_len, '\n');      

    reserve_memory_block(mb_info, os_begin, os_len);

    write_to_serial("\nMemory map (after reserving os):\n");
    show_mmap(mb_info);
    return 1;
}