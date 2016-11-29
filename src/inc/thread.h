#pragma once

#include "list.h"

#define TH_DEAD 0
#define TH_RUNNING 1
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
  uint64_t ret;
} __attribute__((packed));
typedef struct stack_init stack_init_t;

thread_t * thread_create(void (*run)(void *), void * params);
void thread_kill(thread_t * thread);

void switch_threads();
