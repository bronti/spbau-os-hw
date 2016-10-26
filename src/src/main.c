static void qemu_gdb_hang(void)
{
#ifdef DEBUG
	static volatile int wait = 1;

	while (wait);
#endif
}

// #include <unistd.h> 

#include "serial.h"
#include "mmap.h"
#include "interruptions.h"
#include "timer.h"
#include "multiboot_info.h"
#include <desc.h>
#include <ints.h>

void interrupt(void);

void main(mb_info_t * mb_info)
{
    // write_num_to_serial((uint64_t)(uintptr_t)mb_info, '\n');    

	qemu_gdb_hang();

    init_serial();

    init_idt();
    init_pic();
    enable_ints();

    handle_mmap(mb_info);

    // init_pit();
    // mask_pic(0xfe, 1);

    write_to_serial("I'm still alive!\n");
	while (1);
}
