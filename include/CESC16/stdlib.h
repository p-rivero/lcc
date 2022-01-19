#ifndef __STDLIB
#define __STDLIB

// Definitions
#ifndef NULL
#define NULL ((void*)0)     /* Null pointer.  */
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */


/* Converts the string pointed to, by the argument str to an integer (type int).  */
extern int atoi(const char *str);

/* Return a random integer between 0 and INT_MAX inclusive.  */
extern int rand();

/* Seed the random number generator with the given number.  */
extern void srand(unsigned int __seed);

/* Exit the program with an exit code.  */
extern void exit(int code);

/* Exit the program with an exit code.  */
extern void exit(int code);

/* Exit the program with an exit code.  */
extern void exit(int code);

/* Allocate space for n items with a given size each.  */
extern void *calloc(size_t n, size_t size);

#endif
