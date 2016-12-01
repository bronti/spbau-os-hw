#pragma once

#include "list.h"

#define TH_DEAD 0
#define TH_RUNNING 1
#define TH_DYING 2
// #define TH_WAITING 2
// #define TH_SLEEPING 3

struct thread 
{
    // int id;
    list_head_t ll;
    int state;
    uint64_t stack;
    uint64_t stack_end;
    char name[10];
};
typedef struct thread thread_t;

struct stack_init  
{
  // uint64_t flg;
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t r11;
  uint64_t r10;
  uint64_t r9;
  uint64_t r8;
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rbp;
  uint64_t ret;
} __attribute__((packed));
typedef struct stack_init stack_init_t;

void thread_setup();
thread_t * thread_create(void (*run)(void *), void * params);
thread_t * thread_proc_create(void (*run)());
void thread_kill(thread_t * thread);
void thread_wait(thread_t * thread);

void switch_threads();
