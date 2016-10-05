#include "timer.h"
#include "ioport.h"

#define PIT_MASTER_PORT ((unsigned short) 0x43)
#define PIT_DATA_PORT ((unsigned short) 0x40)

void init_pit(void)
{
    out8(PIT_MASTER_PORT, 0x34); 

    out8(PIT_DATA_PORT, 0x0);       // Init = 2^16
    out8(PIT_DATA_PORT, 0x0); 
}