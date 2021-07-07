# LCC Port for the CESC16 Architecture

**[Work in progress]**

## Building the compiler
1. Choose the destination directory for the binaries, and add it to the your console's rc file (`.bashrc` or equivalent):
    - Add the following line to your file: `export LCCDIR=~/lcc`.
    - I recommend `~/lcc`, but other directories can also be used, as long as they don't require root permissions.
2. Create the destination directory: `mkdir $LCCDIR`.
3. Make sure that all `.md` files use LF line termination instead of CLRF. Git will sometimes convert all line endings to CLRF automatically.
4. Run `make all`. The compiler should finish with warnings but no errors.
5. (Recommended) Create an alias (add to `.bashrc` or equivalent): `alias lcc="$LCCDIR/lcc"`
6. (Optional) Update the manfiles `cp doc/*.1 /usr/local/man/man1`

**Some possible errors:**
- Several errors like `line 1: invalid character '%'`: Make sure that all `.md` files use LF line termination.
- Errors when compiling `gram.c`: Make sure it's not been automatically overwritten. Download `lburg/gram.c` again from GitHub if needed.

## Using the compiler
*TODO*

# Original README from the LCC repository
```
This hierarchy is the distribution for lcc version 4.2.

lcc version 3.x is described in the book "A Retargetable C Compiler:
Design and Implementation" (Addison-Wesley, 1995, ISBN 0-8053-1670-1).
There are significant differences between 3.x and 4.x, most notably in
the intermediate code. For details, see
http://storage.webhop.net/documents/interface4.pdf.

VERSION 4.2 IS INCOMPATIBLE WITH EARLIER VERSIONS OF LCC. DO NOT
UNLOAD THIS DISTRIBUTION ON TOP OF A 3.X DISTRIBUTION.

LOG describes the changes since the last release.

CPYRIGHT describes the conditions under you can use, copy, modify, and
distribute lcc or works derived from lcc.

doc/install.html is an HTML file that gives a complete description of
the distribution and installation instructions.

Chris Fraser / cwf@aya.yale.edu
David Hanson / drh@drhanson.net
$Revision$ $Date$
```

