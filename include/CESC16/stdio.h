#ifndef __STDIO
#define __STDIO

#include "stdarg.h"

#ifndef NULL
#define NULL 0
#endif


/* Writes formatted output to the screen.  */
extern void printf(const char *format, ...);

/* Writes formatted output to a string pointed by str.  */
extern void sprintf(char *str, const char *format, ...);

/* Writes formatted output to the screen, using an argument list passed to it.  */
extern int vprintf(const char *format, va_list arg);



/* Outputs a character to the screen.  */
extern int putchar(char c);

/* Outputs a string to the screen, followed by a newline.  */
extern int puts(const char *str);



/* Reads and returns a character from input.
   If no character is available, execution is blocked until a key is pressed.
   Return value: an ASCII character  */
extern char getchar();

/* Reads a line from input and stores it into the string pointed to by str.
   It stops when either the newline character is read or when the end-of-file
   is reached, whichever comes first.
   If there is no available line, execution is blocked until the user presses Enter.
   Return value: NULL if there was an error. str otherwise.  */
extern char *gets(char *str);



#endif
