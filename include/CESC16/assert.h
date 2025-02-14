#undef assert
#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
extern int _assert_failed(char *, char *, unsigned);
#define assert(e) ((void)((e)||_assert_failed(#e, __FILE__, __LINE__)))
#endif /* NDEBUG */
