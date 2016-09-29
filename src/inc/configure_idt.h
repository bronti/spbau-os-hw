#pragma once
#include <stdint.h>
#include <desc.h>

#define IDT_SIZE 48

extern uint64_t table[];
extern struct desc_table_ptr idt_desc;

void c_handler(void);
void init_idt(void);
void init_controller(void);
