#include "configure_idt.h"
#include "serial.h"
#include <stdlib.h>
#include "memory.h"

uint16_t the_table[8 * IDT_SIZE];

void c_handler(void) 
{ 
    write_to_serial("c_handler called\n");
    // while (1);
}

void set_entry(int i, int is_trap, uint64_t dpl)
{
    uint64_t offset = table[i];
    uint64_t gate_type = is_trap ? 15 : 14;

    the_table[8 * i] = (uint16_t) offset;                      // offset [0, 16)
    the_table[8 * i + 1] = KERNEL_CS;                          // segment selector

    the_table[8 * i + 2] |= (1 << 15);                         // P
    the_table[8 * i + 2] |= (dpl << 13);                       // DPL
    the_table[8 * i + 2] |= (gate_type << 8);                  // TYPE   
    the_table[8 * i + 3] = (uint16_t) (offset >> 16);          // offset [16, 32)

    the_table[8 * i + 4] = (uint16_t) (offset >> 32);          // offset [32, 48)
    the_table[8 * i + 5] = (uint16_t) (offset >> 48);          // offset [48, 64)
}

void init_idt(void)
{
    for (int i = 0; i < IDT_SIZE; ++i)
    {
        // set_entry(i, 0, (i < 32 ? 3 : 0));
        set_entry(i, 0, 0);
    }
    struct desc_table_ptr idt_desc = {2 * IDT_SIZE * sizeof(uint64_t) - 1, (uint64_t) the_table};
    write_idtr(&idt_desc);
}