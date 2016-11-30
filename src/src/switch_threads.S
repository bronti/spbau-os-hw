    .text

    .extern thread_wrapper
    .global switch_threads_s
    .global initial_thread_setup


initial_thread_setup:

    movq %r15, %rdi
    movq %r14, %rsi
    cld
    callq thread_wrapper


switch_threads_s:

    pushq %rbp
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %rax
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %rsp, (% rdi)
    movq %rsi, %rsp

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rax
    popq %rbx
    popq %rcx
    popq %rdx
    popq %rbp

    retq
