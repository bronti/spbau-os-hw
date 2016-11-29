#include "alloc.h"
#include "thread.h"
#include "memory.h"
#include "print.h"
#include "ints.h"
#include "time.h"
#include "i8259a.h"

extern void initial_thread_setup();
extern void switch_threads_s(void ** curr, void * next);

thread_t * threads_head = 0;  
int all_started = 0;  
int all_proc_ended = 0;  


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
    thread_t * new_thread = mem_alloc(sizeof(threads_head));
    void * stack = mem_alloc(PAGE_SIZE);
    new_thread->stack = (uint64_t)stack + PAGE_SIZE;

    new_thread->state = TH_RUNNING;
    new_thread->stack_end = (uint64_t)stack;
    thread_add(new_thread);

    return new_thread;
}

void thread_wrapper(void (*run)(void *), void * params)
{
    printf("wrapper reached\n");
    // end of int
    pic_ack(PIT_IRQ);

    enable_ints();
    run(params);

    disable_ints();
    thread_kill(threads_head);
    // enable_ints();

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
    printf("New thread created. %d ", initial_stack);
    printf("%d\n", new_thread->stack);
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
        list_del(&(th->ll));
    }
}

void thread_kill(thread_t * thread)
{
    printf("Eggsterminate!");
    disable_ints();

    thread->state = TH_DEAD;

    switch_threads();

    // enable_ints();
}

void thread_wait(thread_t * thread)
{
    while(thread->state != TH_DEAD) {}
    if ((thread_t *)thread->stack_end)
        mem_free((thread_t *)thread->stack_end);
    mem_free(thread);
}

void signal()
{
    int x;
    printf("pr:%d;x:%d;prret:%d;\n", ((thread_t *)(threads_head->ll.prev))->stack, &x, 
        ((uint64_t *)((thread_t *)(threads_head->ll.prev))->stack)[2]);
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
    if (!all_started)
    {
        // add thread for kernel
        // todo: move to init
        thread_t * kernel = mem_alloc(sizeof(threads_head));
        kernel->state = TH_RUNNING;
        kernel->stack_end = 0;
        for (int i = 0; i < 7; ++i)
        {
            kernel->name[i] = "kernel"[i];      
        }
        thread_add(kernel);
        all_started = 1;
    }
    if (threads_head == (thread_t *)(threads_head->ll.next))
    {        
        printf("one thread detected\n");
        enable_ints();
        return;
    }

    thread_t * curr = threads_head;
    threads_head = (thread_t *)(threads_head->ll.next);
    thread_t * next = threads_head;

    if (curr->state == TH_DEAD)
    {
        thread_del(curr);
    }

    uint64_t * curr_stack = &(curr->stack);
    void * next_stack = (void *)next->stack;
    printf("all ok ");    

    printf("cu:%d;%s;curet:%d;", 
        next_stack, next->name, 
        ((uint64_t *)next_stack)[2]);
    switch_threads_s((void **)curr_stack, next_stack);
    // switch_threads_s((void **)&(((thread_t *)(threads_head->ll.prev))->stack), next_stack);
    // printf("%d\n", *curr_stack);

    int x; 
    printf("cu:%d\n", *curr_stack);
    printf("\ngotcha:%d\n", &x);

    enable_ints();
}