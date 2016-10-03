static void qemu_gdb_hang(void)
{
#ifdef DEBUG
	static volatile int wait = 1;

	while (wait);
#endif
}

#include "serial.h"
#include "interruptions.h"
#include "timer.h"
#include <desc.h>
#include <ints.h>

void interrupt(void);

void main(void)
{
	qemu_gdb_hang();

    init_serial();
    write_to_serial("Hello World!\n");

    init_idt();
    init_pic();
    enable_ints();

    init_pit();
    mask_pic(0xfe, 1);

    // __asm__ ("int $0" : :);                      // test interruptions
    // __asm__ ("int $18" : :);                     // test interruptions
    // int m = 0;
    // int n = 4/ m;
    // n = m;
    // m = n;
    write_to_serial("I'm alive!\n");

	while (1);
}
