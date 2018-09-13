#include "print.h"

#include <stdarg.h>
#include <string.h>
#include <math.h>

#define DEBUG

#define INT_DIGITS 19		/* enough for 64 bit integer */

void send_command(int command, void *message) {
#ifdef DEBUG
    __asm("mov r0, %[cmd];"
          "mov r1, %[msg];"
          "bkpt #0xAB"
            :
            : [cmd] "r" (command), [msg] "r" (message)
              : "r0", "r1", "memory");
#endif
}

char *itoa(int i)
{
	/* Room for INT_DIGITS digits, - and '\0' */
	static char buf[INT_DIGITS + 2];
	char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
	if (i >= 0) {
		do {
			*--p = '0' + (i % 10);
			i /= 10;
		} while (i != 0);
		return p;
	}
	else {			/* i < 0 */
		do {
			*--p = '0' - (i % 10);
			i /= 10;
		} while (i != 0);
		*--p = '-';
	}
	return p;
}

void prints( const char* str ) {

	uint32_t len = strlen(str);
	uint32_t i = 0;
    
	for( i = 0; i < len; i+=4) {

		uint32_t buflen = 4;
		if( i+4 >= len ) buflen = len-i;
		size_t m[] = { 2/*stderr*/, (size_t)str, buflen/sizeof(char) };
		send_command(0x05/* some interrupt ID */, m);
		str+=4;
	}
}

/// Very slow, use prints instead
void printc( char c ) {

	char buf[2];

	buf[0] = c;
    buf[1] = 0;

    prints(buf);
}

void printi( int32_t i ) {

	prints( itoa(i) );
}

char *convert(unsigned int num, int base)
{
	static char Representation[]= "0123456789ABCDEF";
	static char buffer[50];
	char *ptr;

	ptr = &buffer[49];
	*ptr = '\0';

	do
	{
		*--ptr = Representation[num%base];
		num /= base;
	}while(num != 0);

	return(ptr);
}

void print(char* format, ...) {

	char *traverse;
	unsigned int i;
	char *s;
	double d;

	va_list arg;

	va_start(arg, format);
    
    /// Pack buf per 4 characters (last is always NULL)
    char buf[5];
    memset(buf, 0, 5);
    int t = 0;

	for(traverse = format; *traverse != '\0'; traverse++)
	{
		if (*traverse != '%')
		{
            if( t == 5 ) {
                prints(buf);
                memset(buf, 0, 5);
                t = 0;
            }
            
            buf[t%5] = *traverse;
            t++;
			continue;
		}
        
        prints(buf);
        memset(buf, 0, 5);
        t = 0;

		traverse++;

		/// Module 2: Fetching and executing arguments
		switch(*traverse)
		{
        /// Fetch char argument
		case 'c' : i = va_arg(arg,int);
			printc(i);
		break;
        
        /// Fetch Decimal/Integer argument
		case 'd' : i = va_arg(arg,int);
			if(i<0)
			{
				i = -i;
				printc('-');
			}
			prints(convert(i,10));
		break;

        /// Fetch floating point argument
		case 'f' : d = va_arg(arg, double);
			if(d<0)
			{
				d = -d;
				prints("-");
			}
			i = (unsigned int)d;
			prints(convert(i, 10));
			printc('.');

			d = d - i;
			if (d*10.0 < 1.0)
				printc('0');
			if (d*100.0 < 1.0)
				printc('0');
            
            /// Round to 3 decimal places
			prints(convert((int)round(d*1000.0), 10));
		break;

        /// Fetch Octal representation
		case 'o': i = va_arg(arg,unsigned int);
			prints(convert(i,8));
		break;

        /// Fetch string
		case 's': s = va_arg(arg,char *);
			prints(s);
		break;

        /// Fetch Hexadecimal representation
		case 'x': i = va_arg(arg,unsigned int);
			prints(convert(i,16));
		break;
		}
	}
    
    prints(buf);
    memset(buf, 0, 5);

	va_end(arg);
}
