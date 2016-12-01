#include "spinlock.h"
#include "ints.h"

void lock(volatile struct spinlock * lck)
{
    do 
    {
        while(atomic_load_explicit(&lck->locked, memory_order_relaxed) == LOCKED);
    } while(atomic_exchange_explicit(&lck->locked, LOCKED, memory_order_acquire) == LOCKED);
}

void unlock(volatile struct spinlock * lock)
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