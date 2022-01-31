/* x86s running Linux */

#include <string.h>

static char rcsid[] = "$Id$";

#ifndef LCCDIR
#define LCCDIR "/usr/local/lib/lcc/"
#endif

#define DEFAULT_BACKEND "CESC16"

char *suffixes[] = { ".c", ".i", ".s", ".o", ".out", 0 };
char inputs[256] = "";
char *cpp[] = { "/usr/bin/cpp", "-U__GNUC__", /*"-U__STDC__",*/ "$1", "$2", "$3", 0 };
char *include[] = {"-I" LCCDIR "include/" DEFAULT_BACKEND, "-I/usr/include", 0 };
char *com[] = {LCCDIR "rcc", "-target=" DEFAULT_BACKEND, "$1", "$2", "$3", 0 };
char *as[] = { "/usr/bin/cesc16asm", "--no-bin", "-o", "$3", "$1", "$2", 0 };
char *ld[] = { "/usr/bin/cesc16asm", "--link", "-o", "$3", "$1", "$2", 0 };

extern char *concat(char *, char *);

int option(char *arg) {
  	if (strncmp(arg, "-lccdir=", 8) == 0) {
		if (strcmp(cpp[0], LCCDIR "gcc/cpp") == 0)
			cpp[0] = concat(&arg[8], "/gcc/cpp");
		include[0] = concat("-I", concat(&arg[8], "/include/" DEFAULT_BACKEND));
		com[0] = concat(&arg[8], "/rcc");
	} else if (strcmp(arg, "-p") == 0 || strcmp(arg, "-pg") == 0) {
		// Not supported by linker, ignore
	} else if (strcmp(arg, "-b") == 0) 
		;
	else if (strcmp(arg, "-g") == 0)
		;
	else if (strncmp(arg, "-ld=", 4) == 0)
		ld[0] = &arg[4];
	else if (strcmp(arg, "-static") == 0) {
		// Not supported by linker, ignore
	} else
		return 0;
	return 1;
}
