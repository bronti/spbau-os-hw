#pragma once
#include <stdint.h>
#include <desc.h>

#define IDT_SIZE 48

extern uint64_t table[];
extern struct desc_table_ptr idt_desc;

struct interrupt_params
{
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rdi;
    uint64_t rax;

    uint64_t num;
    uint64_t val;
} __attribute__((packed));

void handle_interruption(struct interrupt_params params);
void init_idt(void);
void init_pic(void);
void mask_pic(uint8_t mask, int is_master);
