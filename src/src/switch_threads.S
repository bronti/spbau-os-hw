    .text

    .extern thread_wrapper
    .extern signal
    .global switch_threads_s
    .global initial_thread_setup


initial_thread_setup:

    movq %r15, %rdi
    movq %r14, %rsi
    cld
    callq thread_wrapper


switch_threads_s:

    pushq %r14
    pushq %r15

    movq %rsp, (% rdi)
    movq %rsi, %rsp

    cld 
    callq signal

    popq %r15
    popq %r14

    retq
