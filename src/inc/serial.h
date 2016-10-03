#pragma once 

void init_serial();
int serial_is_busy();
void write_to_serial(char str[]);
void write_i_to_serial(char str[], int count) ;
