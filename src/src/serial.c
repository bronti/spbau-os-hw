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

void write_to_serial(const char str[]) 
{
    for (int i = 0; str[i]; ++i)
    {
        while (serial_is_busy());
        out8(SERIAL_PORT + 0, str[i]);
    }
}

void write_i_to_serial(const char str[], int count) 
{
    for (int i = 0; i < count; ++i)
    {
        while (serial_is_busy());
        out8(SERIAL_PORT + 0, str[i]);
    }
}

void write_num_to_serial(uint64_t num, char ending)//, int base = 10) 
{
    char str_num_buff[20];
    int strlen = 0;
    if (num == 0) 
    {
        str_num_buff[0] = '0';
        strlen = 1;
    }
    else 
    {
        for (; num > 0 && strlen < 20; num /= 10, ++strlen)
        {
            str_num_buff[strlen] = '0' + (num % 10);
        }
    }
    for (; strlen > 0; --strlen)
    {
        write_i_to_serial(&str_num_buff[strlen - 1], 1);
    }
    write_i_to_serial(&ending, 1);
}
