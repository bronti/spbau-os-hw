#include <sys/io.h>
#include "serial.h"

#define SERIAL_PORT ((unsigned short) 0x3f8)

void init_serial() 
{
   outb(0x00, SERIAL_PORT + 1); // disable interruptions
   outb(0x80, SERIAL_PORT + 3); // to set speed in +0 and +1
   outb(0x01, SERIAL_PORT + 0); // low byte of speed  (speed = 115200)
   outb(0x00, SERIAL_PORT + 1); // high byte of speed
   outb(0x03, SERIAL_PORT + 3); // frame size = 8, 1 stop bit 
}

int serial_is_busy() 
{
   return (inb(SERIAL_PORT + 5) & 0x20) == 0;
}

void write_to_serial(char str[]) 
{
    for (int i = 0; str[i]; ++i)
    {
        while (serial_is_busy());
        outb(str[i], SERIAL_PORT + 0);
    }
}
