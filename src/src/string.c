#include "string.h"

void memmove(void * dest, void * src, uint64_t len)
{
    char * where = dest;
    char * from = src;

    if (from >= where || from + len <= where)
    {
        for (uint64_t i = 0; i < len; ++i)
        {
            where[i] = from[i];
        }
    } 
    else
    {        
        for (uint64_t i = len; i > 0; --i)
        {
            where[i - 1] = from[i - 1];
        }
    }
}