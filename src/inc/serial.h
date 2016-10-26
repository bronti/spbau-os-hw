#pragma once 

#include <stdint.h>

void init_serial();
int serial_is_busy();
void write_to_serial(char str[]);
void write_i_to_serial(char str[], int count);
void write_num_to_serial(uint64_t num, char ending);
