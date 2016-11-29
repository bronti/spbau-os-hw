#include "spinlock.h"
#include "ints.h"

void lock(struct spinlock * lock)
{
    do 
    {
        while(atomic_load_explicit(&lock->locked, memory_order_relaxed) == LOCKED);
    } while(atomic_exchange_explicit(&lock->locked, LOCKED, memory_order_acquire) == LOCKED);
}

void unlock(struct spinlock * lock)
{
    atomic_store_explicit(&lock->locked, UNLOCKED, memory_order_release);
}

void global_lock()
{
    disable_ints();
}

void global_unlock()
{
    enable_ints();
}