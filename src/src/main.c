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
    disable_ints();

    init_serial();
    write_to_serial("Hello World!\n");

    init_idt();

    __asm__ ("int $0" : :);                      // test interruptions
    write_to_serial("Interruption tested.\n");
    

	while (1);
}
