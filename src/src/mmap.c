#include "mmap.h"
#include "multiboot_info.h"
#include <stdint.h>
#include "serial.h"

int check_flag(uint32_t flags, int bit)
{
    return (flags & (1 << bit)) != 0;
}

void show_mmap(mb_info_t * mb_info) 
{
    if (check_flag(mb_info->flags, MMAP_FLAG))
    {
        mb_memory_map_t *mmap;

        write_to_serial("\nMemory map:\n");
        
        write_to_serial("\naddr: ");
        write_num_to_serial((uint64_t) mb_info->mmap_addr, ' ');
        write_to_serial("\nlength: ");
        write_num_to_serial((uint64_t) mb_info->mmap_length, '\n');

        for(mmap = (mb_memory_map_t *) ((uint64_t) mb_info->mmap_addr);
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