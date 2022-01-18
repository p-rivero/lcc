#ifndef __STDARG
#define __STDARG

#if !defined(_VA_LIST) && !defined(__VA_LIST_DEFINED)
#define _VA_LIST
#define _VA_LIST_DEFINED
typedef char *__va_list;
#endif
static float __va_arg_tmp;
typedef __va_list va_list;

#define va_start(list, start) ((void)((list) = (va_list)(&(start)+1)))
	
#define __va_arg(list, mode) (*(mode *)(list++))

#define va_end(list) ((void)0)

#define va_arg(list, mode) __va_arg(list, mode)

typedef void *__gnuc_va_list;

#endif
