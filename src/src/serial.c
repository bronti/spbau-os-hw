// #include <sys/io.h>
#include "serial.h"
#include "ioport.h"

#define SERIAL_PORT ((unsigned short) 0x3f8)

void init_serial() 
{
   out8(SERIAL_PORT + 1, 0x00); // disable interruptions
   out8(SERIAL_PORT + 3, 0x80); // to set speed in +0 and +1
   out8(SERIAL_PORT + 0, 0x01); // low byte of speed  (speed = 115200)
   out8(SERIAL_PORT + 1, 0x00); // high byte of speed
   out8(SERIAL_PORT + 3, 0x03); // frame size = 8, 1 stop bit 
}

int serial_is_busy() 
{
   return (in8(SERIAL_PORT + 5) & 0x20) == 0;
}

void write_to_serial(char str[]) 
{
    for (int i = 0; str[i]; ++i)
    {
        while (serial_is_busy());
        out8(SERIAL_PORT + 0, str[i]);
    }
}

void write_i_to_serial(char str[], int count) 
{
    for (int i = 0; i < count; ++i)
    {
        while (serial_is_busy());
        out8(SERIAL_PORT + 0, str[i]);
    }
}
