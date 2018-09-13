#ifndef PRINT__H_
#define PRINT__H_

#include <stdint.h>

void print(char* format, ...);

void printc( char c );

void printi( int32_t i );

void prints( const char* str );

void send_command(int command, void *message);

#endif
