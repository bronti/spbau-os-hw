static void qemu_gdb_hang(void)
{
#ifdef DEBUG
	static volatile int wait = 1;

	while (wait);
#endif
}

#include "serial.h"
#include "configure_idt.h"
#include <desc.h>
#include <ints.h>

void interrupt(void);

void main(void)
{
	qemu_gdb_hang();

    init_serial();
    write_to_serial("Hello World!\n");

    init_idt();
    disable_ints();

    // __asm__ ("int $0" : :); // test interruptions
    

	while (1);
}
