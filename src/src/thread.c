#include "alloc.h"
#include "thread.h"
#include "memory.h"
#include "print.h"
#include "ints.h"
#include "time.h"
#include "i8259a.h"
#include "debug.h"

extern void initial_thread_setup();
extern void switch_threads_s(void ** curr, void * next);

thread_t * threads_head = 0;  
int all_started = 0;  
int all_proc_ended = 0;  

#define THREAD_STACK_SIZE 2 * PAGE_SIZE


void thread_add(thread_t * th) 
{
    if (threads_head == 0)
    {
        threads_head = th;
        list_init(&(th->ll));
    }
    else
    {
        list_add_tail(&(th->ll), &(threads_head->ll));
    }
}

thread_t * new_thread_init()
{
    thread_t * new_thread = mem_alloc(sizeof(*new_thread));
    void * stack_end = mem_alloc(THREAD_STACK_SIZE);
    new_thread->stack = (uint64_t)stack_end + THREAD_STACK_SIZE;

    new_thread->state = TH_RUNNING;
    new_thread->stack_end = (uint64_t)stack_end;
    thread_add(new_thread);

    return new_thread;
}

void thread_setup()
{
    // disable_ints();
    // add thread for kernel
    thread_t * kernel = mem_alloc(sizeof(*kernel));
    kernel->state = TH_RUNNING;
    kernel->stack_end = 0;
    kernel->stack = 0;
    thread_add(kernel);
    all_started = 1;
    for (int i = 0; i < 7; ++i)
    {
        kernel->name[i] = "kernel"[i];      
    }    
    // enable_ints();
}

void thread_wrapper(void (*run)(void *), void * params)
{
    // printf("Wrapper reached.\n");
    pic_ack(PIT_IRQ);

    enable_ints();
    run(params);

    disable_ints();
    thread_kill(threads_head);
    enable_ints();

    printf("Beware zombie thread! Oo");
}

thread_t * thread_create(void (*run)(void *), void * params)
{
    disable_ints();

    thread_t * new_thread = new_thread_init(); 

    new_thread->stack -= sizeof(stack_init_t);
    stack_init_t * initial_stack = (stack_init_t *)new_thread->stack;
    initial_stack->ret = (uint64_t)&initial_thread_setup;
    initial_stack->r15 = (uint64_t)run;
    initial_stack->r14 = (uint64_t)params;

    if (threads_head == (thread_t *)((thread_t *)threads_head->ll.next)->ll.next)
    {
        for (int i = 0; i < 6; ++i)
        {
            new_thread->name[i] = "first" [i]    ;       
        }
    }
    else
    {
        for (int i = 0; i < 7; ++i)
        {
            new_thread->name[i] = "second" [i]  ;         
        }
    }

    // printf("New thread created.\n");
    enable_ints();
    return new_thread;
}

void thread_del(thread_t * th) 
{
    if (threads_head == th)
    {
        threads_head = (thread_t *)threads_head->ll.next;
    }

    if (threads_head == th)
    {
        all_proc_ended = 1;
    }
    else
    {
        list_del((list_head_t *)th);        
        printf("Thread deleted.\n");
    }
}

void thread_kill(thread_t * thread)
{
    // printf("Eggsterminate!");
    disable_ints();

    if (threads_head == thread)
    {
        thread->state = TH_DYING;
        switch_threads();
    }
    else
    {
        thread_del(thread);
        // printf("Going to make headshot (from outer).\n");
        thread->state = TH_DEAD;
        // printf("Headshot (from outer).\n");
    }

    enable_ints();
}

void thread_wait(thread_t * thread)
{
    while(thread->state != TH_DEAD);
    printf("Waiting succeeded on %s, %d.\n", thread->name, (uint64_t)thread);
    BUG_ON(threads_head == thread);
    if (thread->stack_end)
        mem_free((void *)(thread->stack_end));
    mem_free(thread);
}

void switch_threads()
{
    disable_ints();
    if (all_proc_ended || (all_started && threads_head == 0))
    {
        all_proc_ended = 1;
        enable_ints();
        return;
    }
    BUG_ON(!all_started)
    if (threads_head == (thread_t *)(threads_head->ll.next))
    {        
        // printf("One thread detected.\n");
        enable_ints();
        return;
    }

    thread_t * curr = threads_head;
    threads_head = (thread_t *)(curr->ll.next);
    thread_t * next = threads_head;

    if (curr->state == TH_DEAD )
    {
        printf("curr: %s\n", curr->name);
        BUG("");
    }
    if (!next)
    {
        printf("curr: %s\n", curr->name);
        BUG("");
    }
    if (next->state == TH_DEAD)
    {
        printf("next: %s\n", curr->name);
        // printf("curr: %s\n", curr->name);
        // // printf("th: %s\n", curr-name);
        // printf("prv: %s\n", ((thread_t *)(curr->ll.prev))->name);
        // printf("prvprv: %s\n", ((thread_t *)(((thread_t *)(curr->ll.prev))->ll.prev))->name);
        // printf("prvnxt: %s\n", ((thread_t *)(((thread_t *)(curr->ll.prev))->ll.next))->name);

        // printf("nxt: %s\n", ((thread_t *)(curr->ll.next))->name);
        // printf("nxtnxt: %s\n", ((thread_t *)(((thread_t *)(curr->ll.next))->ll.next))->name);
        // printf("nxtprv: %s\n", ((thread_t *)(((thread_t *)(curr->ll.next))->ll.prev))->name);
        BUG("");
    }
    if (curr->state == TH_DYING)
    {
        thread_del(curr);

        // printf("Going to make headshot (from switch) to %s, %d.\n", curr->name, (uint64_t)curr);
        curr->state = TH_DEAD;
        // printf("Headshot (from switch).\n");
    }

    uint64_t * curr_stack = &(curr->stack);
    void * next_stack = (void *)next->stack;

    printf("!");
    switch_threads_s((void **)curr_stack, next_stack);

    enable_ints();
}