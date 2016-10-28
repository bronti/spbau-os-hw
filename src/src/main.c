static void qemu_gdb_hang(void)
{
#ifdef DEBUG
	static volatile int wait = 1;

	while (wait);
#endif
}

// #include <unistd.h> 

#include "serial.h"
#include "paging.h"
#include "buddy_allocator.h"
#include "mmap.h"
#include "interruptions.h"
#include "timer.h"
#include "multiboot_info.h"
#include <desc.h>
#include <ints.h>

void interrupt(void);

uint64_t to_phys(uint64_t addr);

void main(mb_info_t * mb_info)
{
    // write_num_to_serial((uint64_t)(uintptr_t)mb_info, '\n');    

	qemu_gdb_hang();

    init_serial();

    init_idt();
    init_pic();
    enable_ints();

    if (!handle_mmap(mb_info))
    {
        write_to_serial("Something wrong with mmap.");
        return;
    }

    // init_pit();
    // mask_pic(0xfe, 1);

    // init_paging(mb_info);
    if (!init_buddy_allocator(mb_info))
    {
        write_to_serial("Something wrong with buddy.");
        return;
    }
    // show_buddies();

    write_to_serial("I'm still alive!\n");
	while (1);
}
