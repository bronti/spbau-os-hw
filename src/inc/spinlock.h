#include<stdatomic.h>

#define LOCKED 1
#define UNLOCKED 0

struct spinlock
{
    atomic_int locked;
};

void lock(struct spinlock * lock);
void unlock(struct spinlock * lock);

void global_lock();
void global_unlock();