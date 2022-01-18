#ifndef __LIMITS
#define __LIMITS

#define CHAR_BIT	16
#define MB_LEN_MAX	1

#define UCHAR_MAX	0xffff
#define USHRT_MAX	0xffff
#define UINT_MAX	0xffff
#define ULONG_MAX	0xffff

#define SCHAR_MAX	0x7fff
#define SHRT_MAX	0x7fff
#define INT_MAX		0x7fff
#define LONG_MAX	0x7fff

#define SCHAR_MIN	(-SCHAR_MAX-1)
#define SHRT_MIN	(-SHRT_MAX-1)
#define INT_MIN		(-INT_MAX-1)
#define LONG_MIN	(-LONG_MAX-1)

#define CHAR_MAX	SCHAR_MAX
#define CHAR_MIN	SCHAR_MIN

#endif /* __LIMITS */
