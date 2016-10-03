#include "interruptions.h"
#include "serial.h"
#include "memory.h"
#include "ioport.h"

uint16_t the_table[8 * IDT_SIZE];

#define MASTER_COMMAND_PORT ((unsigned short) 0x20)
#define MASTER_DATA_PORT    ((unsigned short) 0x21)
#define SLAVE_COMMAND_PORT  ((unsigned short) 0xA0)
#define SLAVE_DATA_PORT     ((unsigned short) 0xA1)

// void handle_interruption(void)
void handle_interruption(struct interrupt_params params)
{ 
    uint64_t num = params.num;
    char str_num_buff[3];
    int strlen = 0;
    if (num == 0) 
    {
        str_num_buff[2] = '0';
        strlen = 1;
    }
    else 
    {
        int i = 2;
        for (; num > 0 && i >= 0; num /= 10, --i)
        {
            str_num_buff[i] = '0' + (num % 10);
            ++strlen;
        }
    }
    write_to_serial("interruption handler no ");
    int i = 2 - strlen + 1;
    for (; i <= 2; ++i)
    {
        write_i_to_serial(&str_num_buff[i], 1);
    }
    write_to_serial(" called\n");
    if (params.num >= 32 && params.num < 40)                  // end of interrupt
    {
        out8(MASTER_COMMAND_PORT, 0x20);
    }
    else if (params.num >= 40)
    {
        out8(MASTER_COMMAND_PORT, 0x20); 
        out8(SLAVE_COMMAND_PORT,  0x20);        
    }
}

void set_entry(int i, int is_trap, uint64_t dpl)
{
    uint64_t offset    = table[i];
    uint64_t gate_type = is_trap ? 15 : 14;

    the_table[8 * i]     = (uint16_t) offset;                  // offset [0, 16)
    the_table[8 * i + 1] = KERNEL_CS;                          // segment selector

    the_table[8 * i + 2] |= (1 << 15);                         // P
    the_table[8 * i + 2] |= (dpl << 13);                       // DPL
    the_table[8 * i + 2] |= (gate_type << 8);                  // TYPE   
    the_table[8 * i + 3] =  (uint16_t) (offset >> 16);         // offset [16, 32)

    the_table[8 * i + 4] =  (uint16_t) (offset >> 32);         // offset [32, 48)
    the_table[8 * i + 5] =  (uint16_t) (offset >> 48);         // offset [48, 64)
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

void init_pic(void)
{
    out8(MASTER_COMMAND_PORT, 0x11);  // 3b of data, cascade, edge triggered
    out8(SLAVE_COMMAND_PORT, 0x11);   

    out8(MASTER_DATA_PORT, 32);       // first idt entry
    out8(SLAVE_DATA_PORT, 40);

    out8(MASTER_DATA_PORT, 0x4);      // slave port
    out8(SLAVE_DATA_PORT, 2);         // master's port

    out8(MASTER_DATA_PORT, 0x1);      // no idea
    out8(SLAVE_DATA_PORT, 0x1);         

    out8(MASTER_DATA_PORT, 0xff);     // mask it all
    out8(SLAVE_DATA_PORT, 0xff);         
}

void mask_pic(uint8_t mask, int is_master)
{    
    if (is_master)
    {
        out8(MASTER_DATA_PORT, mask);  
    }
    else 
    {
        out8(SLAVE_DATA_PORT, mask);          
    }    
}