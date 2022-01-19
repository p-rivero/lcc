#include "c.h"

enum { R_ZERO=0, R_SP=1, R_BP=2, R_T0=3, R_T1=4, R_T2=5, R_T3=6, R_A0=7, R_A1=8, R_A2=9, R_A3=10, R_S0=11, R_S1=12, R_S2=13, R_S3=14, R_S4=15, R_ret=16};
// R_ret is a fake register with the same name as R_A0. This way lcc can overwrite a0 without complaining.
static char *regnames[] = {"zero", "sp", "bp", "t0", "t1", "t2", "t3", "a0", "a1", "a2", "a3", "s0", "s1", "s2", "s3", "s4", "a0"};
// Number of elements in the enum
static const int REGFILE_SZ = R_ret + 1;

static const int INTTMP = (1<<R_T0)|(1<<R_T1)|(1<<R_T2)|(1<<R_T3);
static const int INTVAR = (1<<R_S0)|(1<<R_S1)|(1<<R_S2)|(1<<R_S3)|(1<<R_S4);
static const int INTARG = (1<<R_A1)|(1<<R_A2)|(1<<R_A3);
static const int INTRET = (1<<R_ret);

static const int ARGREG_AMOUNT = R_A3 - R_A0 + 1;
static const int SAFEREG_AMOUNT = R_S4 - R_S0 + 1;

// Stats required for determining whether a variable must go on a register
static int total_amount = 0;
static int locals_amount = 0;
static int refs_amount = 0;
static char has_addressed = 0;



static void address(Symbol, Symbol, long);
static void blkfetch(int, int, int, int);
static void blkloop(int, int, int, int, int, int[]);
static void blkstore(int, int, int, int);
static void defaddress(Symbol);
static void defconst(int, int, Value);
static void defstring(int, char *);
static void defsymbol(Symbol);
static void doarg(Node);
static void emit2(Node);
static void export(Symbol);
static void clobber(Node);
static void function(Symbol, Symbol [], Symbol [], int);
static void global(Symbol);
static void import(Symbol);
static void local(Symbol);
static void progbeg(int, char **);
static void progend(void);
static void segment(int);
static void space(int);
static void target(Node);
static int memop(Node);
static int sametree(Node, Node);
static Symbol argreg(int);

static Symbol intreg[32];
static Symbol intregw;

static int cseg;

// Autogenerated by lburg
static void _kids(Node, int, Node[]);
static void _label(Node);
static int _rule(void*, int);
static short *_nts[];
static char *_string[];
static char *_templates[];
static char _isinstruction[];
static char *_ntname[];

static void progbeg(int argc, char *argv[]) {    
    int i;
    {
        union {
            char c;
            int i;
        } u;
        u.i = 0;
        u.c = 1;
        swap = ((int)(u.i == 1)) != IR->little_endian;
    }
    parseflags(argc, argv);
    
    assert(sizeof(regnames)/sizeof(regnames[0]) == REGFILE_SZ);
    // Create register bank
    for (i = 0; i < REGFILE_SZ; i++)
        intreg[i]  = mkreg(regnames[i], i, 1, IREG);
    
    intregw = mkwildcard(intreg);
    
    tmask[IREG] = INTTMP;
    vmask[IREG] = INTVAR;
    
    cseg = 0; // Don't start at any specific bank
}


static Symbol rmap(int opk) {
    switch (optype(opk)) {
    case B: case P: case I: case U:
        return intregw;            
    case F:
        error("Floats are not supported\n", 0);
        exit(EXIT_FAILURE);
    default:
        return 0;
    }
}


static void segment(int n) {
    if (n == cseg) return;
    cseg = n;
    if (cseg == CODE || cseg == LIT)
        print("\n#bank program\n");
    else if (cseg == DATA || cseg == BSS)
        print("\n#bank data\n");
}


static void progend(void) {

}

int memop(Node p) {
    assert(p);
    assert(generic(p->op) == ASGN);
    assert(p->kids[0]);
    assert(p->kids[1]);
    if (generic(p->kids[1]->kids[0]->op) == INDIR && sametree(p->kids[0], p->kids[1]->kids[0]->kids[0]))
        return 3;
    else
        return LBURG_MAX;
}
int sametree(Node p, Node q) {
    if (p == NULL && q == NULL)
        return 1;
    if (p && q && p->op == q->op && p->syms[0] == q->syms[0])
        return (sametree(p->kids[0], q->kids[0]) && sametree(p->kids[1], q->kids[1]));
}


// Given an ARG node, find its corresponding CALL node and return the offset
static Node find_CALL(Node p, int *offset) {
    assert(generic(p->op) == ARG);
    *offset = 0;
    Node funct = p;
    
    // p->link points at the next node. The root should be the called function 
    while (generic(funct->op) != CALL) {
        // Reached the root
        if (funct->link == NULL) {
            break;
        }
        // "If wants_dag=0, only CALLV appears as root; other CALL nodes appear as right operands to ASGN" funct->kids[1]->kids[0]->op
        if (generic(funct->op) == ASGN && generic(funct->kids[1]->op) == CALL) {
            funct = funct->kids[1];
            break;
        }
        // Previous condition is only triggered on target(), this works on emit2()
        if (generic(funct->op) == ASGN && generic(funct->kids[1]->op) == LOAD && generic(funct->kids[1]->kids[0]->op) == CALL) {
            funct = funct->kids[1]->kids[0];
            break;
        }
        
        funct = funct->link;
        if (generic(funct->op) == ARG) (*offset)++; // Found a new ARG: increment offset
    }
    assert(generic(funct->op) == CALL);
    return funct;
}

static int funct_is_variadic(Node funct) {
    if (funct->syms[1]) {
        assert(funct->syms[1]->type->op == FUNCTION);
        return variadic(funct->syms[1]->type);
    }
    else {
        assert(funct->syms[0]->type->op == FUNCTION);
        return variadic(funct->syms[0]->type);
    }
}

static void target(Node p) {
    assert(p);
    switch (specific(p->op)) {
    case MUL+I: case MUL+U: case DIV+I: case DIV+U: case MOD+I: case MOD+U:
        setreg(p, intreg[R_A0]);        // Result location
        rtarget(p, 0, intreg[R_A0]);    // Where to store arg
        rtarget(p, 1, intreg[R_A1]);    // Where to store arg
        break;
    
    case LSH+I: case LSH+U: case RSH+I: case RSH+U:
        assert(p->kids[1]);
        if (generic(p->kids[1]->op) == CNST) break;
        setreg(p, intreg[R_A0]);     // Result location
        rtarget(p, 0, intreg[R_A0]); // Where to store src
        rtarget(p, 1, intreg[R_A1]); // Where to store shamt
        break;
        
    case CNST+I: case CNST+U: case CNST+P:
        // Use zero register for constants that are 0
        if (range(p, 0, 0) == 0) {
            setreg(p, intreg[R_ZERO]); 
            p->x.registered = 1;
        }
        break;
        
    case ASGN+B:
        //rtarget(p, 0, intreg[R_T1]); // Where to store destination address
        //rtarget(p->kids[1], 0, intreg[R_T0]); // Where to store source address
        break;
        
    case CVF+I:
        setreg(p, intreg[R_A0]); // Result location
        break;
        
    case CALL+I: case CALL+U: case CALL+P: case CALL+V:
        setreg(p, intreg[R_A0]); // Result location
        break;
        
    case RET+I: case RET+U: case RET+P:
        rtarget(p, 0, intreg[R_ret]); // Where to store ret value
        break;
        
    case ARG+I: case ARG+U: case ARG+P: {
        // Arguments are computed right to left: the LAST (leftmost) 4 args may be passed in registers
        int offset;
        Node funct = find_CALL(p, &offset);
        
        // Once the function node is reached, see if it's variadic
        int is_variadic = funct_is_variadic(funct);
        Symbol r = argreg(offset);
        
        // If this arg is passed in a register, store the data in that register
        if (r != NULL && !is_variadic) rtarget(p, 0, r);
        break;
        }
    }
}

static void clobber(Node p) {
    assert(p);
    switch (specific(p->op)) {        
    case ASGN+B:
        // t0 is used as the temporary location for copied data.
        // If block copy requires more regs, spill them here.
        spill((1<<R_T0), IREG, p);
        break;
        
    case CALL+I: case CALL+U: case CALL+P:
        spill(INTTMP, IREG, p);
        break;
        
    case CALL+V:
        spill(INTTMP|INTRET, IREG, p);
        break;
        
    case MUL+I: case MUL+U:
        spill(INTARG, IREG, p);
        break;
        
    case DIV+U: case MOD+U:
        // Unsigned div overwrites t0
        spill(INTARG|(1<<R_T0), IREG, p);
        break;
        
    case DIV+I: case MOD+I:
        // Signed div overwrites t0, t1 and t2
        spill(INTARG|(1<<R_T0)|(1<<R_T1)|(1<<R_T2), IREG, p);
        break;
        
    case LSH+I: case LSH+U: case RSH+I: case RSH+U:
        assert(p->kids[1]);
        if (generic(p->kids[1]->op) != CNST)
            spill(INTARG, IREG, p);
        break;
    }
}


#define nodeC(p) (intreg[getregnum(p)]->x.name)
#define node0(p) (intreg[getregnum(p->x.kids[0])]->x.name)
#define node1(p) (intreg[getregnum(p->x.kids[1])]->x.name)
#define emitchild(p, N) (emitasm(p->kids[N], _nts[_rule(p->x.state, p->x.inst)][N]))

static void emit2(Node p) {
    int op = specific(p->op);

    if (generic(op) == CVI || generic(op) == CVU || generic(op) == LOAD) {
        char *dst = nodeC(p);
        char *src = node0(p);
        assert(opsize(p->op) <= opsize(p->x.kids[0]->op));
        if (dst != src)
            print("\tmov %s, %s\n", dst, src);
    }
    else if (op == CALL+I || op == CALL+U  || op == CALL+P || op == CALL+V) {
        int is_variadic = funct_is_variadic(p);
        int arg_sz = p->syms[0]->u.c.v.i;
        
        // Call the function
        print("\tcall ");
        emitchild(p, 0);
        print("\n");
        // Non-variadic functions pass the first 4 arguments on registers
        if (!is_variadic) arg_sz -= ARGREG_AMOUNT;
        
        if (arg_sz > 0) print("\tadd sp, sp, %d\n", arg_sz);
    }
    else if (op == ARG+I || op == ARG+U || op == ARG+P) {
        // Arguments are computed right to left: the LAST (leftmost) 4 args may be passed in registers
        // p->link points at the next ARG. The last arg points at the called function 
        int offset;
        Node funct = find_CALL(p, &offset);
        
        // Once the function node is reached, see if it's variadic
        int is_variadic = funct_is_variadic(funct);
        Symbol r = argreg(offset);
        
        // Arguments of a variadic function are always passed in the stack
        if (r == NULL || is_variadic) {
            // Pass in stack
            print("\tpush ");
            emitchild(p, 0);
            print("\n");
        }
        // First 4 arguments: pass in registers
        // If the argument wasn't already stored in the register, emit a mov
        else if (p->x.kids[0] == NULL || p->x.kids[0]->syms[RX] != r) {
            print("\tmov %s, ", r->name);
            emitchild(p, 0);
            print("\n");
        }
    }
    else if (op == ASGN+B) {
        assert(p->syms[0]);
        assert(p->x.kids[0]);
        assert(p->x.kids[1]);
        
        int size = p->syms[0]->u.c.v.i;
        char *dst_reg = p->x.kids[0]->syms[RX]->x.name;
        char *src_reg = p->x.kids[1]->syms[RX]->x.name;
        
        assert(size > 0);
        // The first iteration is different (1 cycle faster)
        print("\tmov t0, [%s]\n", src_reg);
        print("\tmov [%s], t0\n", dst_reg);
        for (int i = 1; i < size; i++) {
            print("\tmov t0, [%s+%d]\n", src_reg, i);
            print("\tmov [%s+%d], t0\n", dst_reg, i);
        }
    }
}

static Symbol argreg(int offset) {
    // Select an argument register, return null if the arg is stored in stack    
    if (offset >= ARGREG_AMOUNT) return NULL;
    else return intreg[R_A0 + offset];
}

static void doarg(Node p) {
    static int argno;
    
    if (argoffset == 0) argno = 0;
    p->x.argno = argno++;
    
    // Store the argument offset in syms[2]
    int align = p->syms[1]->u.c.v.i;
    int offs = mkactual(align, p->syms[0]->u.c.v.i);
    p->syms[2] = intconst(offs);
}

static void blkfetch(int k, int off, int reg, int tmp) {
    print(";! blkfetch");
}
static void blkstore(int k, int off, int reg, int tmp) {
    print(";! blkstore");
}
static void blkloop(int dreg, int doff, int sreg, int soff, int size, int tmps[]) {
    print(";! blkloop");
}


static void local(Symbol p) {
    // Is the new local worth storing in a register?
    if (!askregvar(p, rmap(ttob(p->type))))
        mkauto(p);  // If not, allocate space in the stack
}

static void function(Symbol f, Symbol caller[], Symbol callee[], int n) {
    int i;
    int is_variadic = variadic(f->type);
    
    segment(CODE);
    usedmask[0] = usedmask[1] = 0;
    freemask[0] = freemask[1] = ~(unsigned)0;
    int arg_offset = 2; // Stack offset for fetching arguments. Skip stored bp and ret value
    offset = 0; // Stack offset for local variables. Increment in order to allocate a word.
    int has_temps_or_args = 0; // 1 if the function needs to store bp
    
    for (i = 0; callee[i]; i++) {
        // Assign location for argument i
        Symbol p = callee[i];
        Symbol q = caller[i];
        assert(q);
        p->x.offset = q->x.offset = arg_offset;
        p->x.name = q->x.name = stringf("%d", p->x.offset);
        
        Symbol r = argreg(i);
        
        // On variadic functions, all arguments are stored in stack
        if (is_variadic || r == NULL) {
            p->sclass = q->sclass = AUTO;
            arg_offset += q->type->size; // Increment stack offset
            has_temps_or_args = 1;
        }
        // No calls (that could overwrite the argument) are performed: leave argument in place.
        // (Except if the address of the variable is needed, or if a return value will be stored in a0)
        else if (n == 0 && !p->addressed /* && !(i == 0 && f->type->type->op != VOID && !isstruct(f->type->type)) */) {
            assert(!isstruct(q->type)); // Structs will always be passed as pointers
            p->sclass = q->sclass = REGISTER;
            askregvar(p, r);
            assert(p->x.regnode && p->x.regnode->vbl == p);
            q->x = p->x;
            q->type = p->type;
        }
        // Register will be overwritten, but it's used enough to be moved to another register
        else if (n >= 0 && askregvar(p, rmap(ttob(p->type)))) {
            assert(q->sclass != REGISTER);
            p->sclass = q->sclass = REGISTER;
            q->type = p->type;
        }
        // Register must be stored in memory (using a negative offset)
        else {
            offset++;   // Allocate 1 word in the stack
            p->x.offset = q->x.offset = -offset;    // Local variables have a negative index
            p->x.name = q->x.name = stringf("%d", p->x.offset);
            has_temps_or_args = 1;
        }
    }
    
    assert(caller[i] == 0);
    
    maxoffset = offset;
    gencode(caller, callee);
    
    framesize = maxoffset; // Don't round up the frame size
    usedmask[IREG] &= INTVAR; // Only save the safe registers
    if (framesize > 0) has_temps_or_args = 1;
    
    print("\n%s:\n", f->x.name);
    if (has_temps_or_args) {
        print("\tpush bp\n");
        print("\tmov bp, sp\n");
    }
    
    if (framesize > 0) print("\tsub sp, sp, %d\n", framesize);
    
    // Store safe regs
    for (i = R_S0; i <= R_S4; i++) {
        if (usedmask[IREG] & (1<<i)) {
            print("\tpush %s\n", regnames[i]);
        }
    }
    
    for (i = 0; i < ARGREG_AMOUNT && callee[i]; i++) {
        Symbol r = argreg(i);
        if (r != NULL && r->x.regnode != callee[i]->x.regnode) {
            
            Symbol out = callee[i];
            Symbol in  = caller[i];
            
            int rn = r->x.regnode->number;
            int rs = r->x.regnode->set;
            int tyin = ttob(in->type);

            assert(out && in && r && r->x.regnode);
            assert(out->sclass != REGISTER || out->x.regnode);
            if (out->sclass == REGISTER && (isint(out->type) || out->type == in->type)) {
                int outn = out->x.regnode->number;
                print("\tmov %s, %s\n", regnames[outn], regnames[rn]);
            }
            else if (!is_variadic) {
                assert(out->type->size == 1);
                print("\tmov [bp+%d], %s\n", out->x.offset, r->name);
            }
        }
    }
    
    emitcode();
    
    // Restore safe regs (FIFO: pop in reverse order)
    for (i = R_S4; i >= R_S0; i--) {
        if (usedmask[IREG] & (1<<i)) {
            print("\tpop %s\n", regnames[i]);
        }
    }
    
    if (framesize > 0) print("\tmov sp, bp\n");
    if (has_temps_or_args) print("\tpop bp\n");
    print("\tret\n");
    
    total_amount = 0;
    locals_amount = 0;
    refs_amount = 0;
    has_addressed = 0;
}


static void defsymbol(Symbol p) {
    if (p->scope >= LOCAL && p->sclass == STATIC) {
        p->x.name = stringf(".L%d", genlabel(1));
    }
    else if (p->generated) {
        if (p->scope == GLOBAL || p->sclass == EXTERN) p->x.name = stringf("_GL%s", p->name);
        else p->x.name = stringf(".L%s", p->name);
    }
    else if (p->scope == GLOBAL || p->sclass == EXTERN) {
        p->x.name = stringf("%s", p->name);
    }
    else if (p->scope == CONSTANTS && (isint(p->type) || isptr(p->type)) && p->name[0] == '0' && p->name[1] == 'x') {
        p->x.name = stringf("0x%s", &p->name[2]);
    }
    else {
        p->x.name = p->name;
    }
}


static void address(Symbol q, Symbol p, long n) {
    if (p->scope == GLOBAL || p->sclass == STATIC || p->sclass == EXTERN) {
        q->x.name = stringf("%s%s%D", p->x.name, n >= 0 ? "+" : "", n);
    }
    else {
        assert(n <= INT_MAX && n >= INT_MIN);
        q->x.offset = p->x.offset + n;
        q->x.name = stringd(q->x.offset);
    }
}


static void defconst(int suffix, int size, Value v) {
    assert(size == 1);
    if (suffix == I) print("#d16 %d\n", v.i);
    else if (suffix == U) print("#d16 %d\n", v.u);
    else if (suffix == P) print("#d16 0x%x\n", v.p);
    else if (suffix == F) {
        error("Floats are not supported\n", 0);
        exit(EXIT_FAILURE);
    }
    else assert(0);
}


static void defaddress(Symbol p) {
    print("#d16 %s\n", p->x.name);
}

char buf[2];
char *get_escape_seq(char c) {
    switch (c) {
    case '\a': return "\\a";
    case '\b': return "\\b";
    case '\e': return "\\e";
    case '\f': return "\\f";
    case '\n': return "\\n";
    case '\r': return "\\r";
    case '\t': return "\\t";
    case '\v': return "\\v";
    case '\0': return "\\0";
    default:
        buf[0] = c;
        buf[1] = '\0';
        return buf;
    }
}

static void defstring(int n, char *str) {
    char *s;
    print("#d \"");
    for (s = str; s < str + n; s++) 
        print("%s", get_escape_seq(*s));
    
    print("\"");
    print("\n#align 32");
}


static void export(Symbol p) {
    //print("public %s\n", p->x.name);
}


static void import(Symbol p) {
    if (p->ref > 0) {
        // print("; extern %s\n", p->x.name);
    }
}


static void global(Symbol p) {
    int align_bits = p->type->align*16;
    print("\n");
    if (align_bits > 16) print("#align %d\n", align_bits);
    print("%s:\n", p->x.name);
}


static void space(int n) {
    print("#res %d\n", n);
}

// Gather info about the local variables of a function (called once per variable)
static void info_var(float ref, int addressed) {
    if (addressed) {
        if (ref > 0.0) has_addressed = 1;
    }
    else {
        total_amount++;
        if (ref < 3.0) {
            locals_amount++;
            refs_amount += ref;
        }
    }
}
// Return 1 if the variable should be stored in a register (called once per variable)
static int var_register(float ref) {
    // Push/Pop of a safe register takes 6 cycles
    // Each indexed reference takes 2 extra cycles
    // Therefore, if a variable has 3 or more references it's ALWAYS worth it
    if (ref >= 3.0) return 1;
    
    // Not all variables will be stored in registers. The cost of initializing
    // the frame becomes 0. Therefore, all variables with less than 3 references
    // will be stored in the frame.
    if (total_amount > SAFEREG_AMOUNT || has_addressed) return 0;
    
    // Cycles for storing ALL remaining variables in registers: 6*locals_amount
    // Cycles for storing ALL in frame: 2*refs_amount (+ cost of initializing frame = 13)
    return (6 * locals_amount < (2 * refs_amount + 13));
}


Interface CESC16IR = {
    // Not byte-oriented: integer size is 1 word, each word is 16 bits wide
    16,       /* byte size (in bits) */
    
    // Type metrics: Size, Align and outofline flag (if outofline=1, constants cannot appear in dags)
    1, 1, 0,  /* char */
    1, 1, 0,  /* short */
    1, 1, 0,  /* int */
    1, 1, 0,  /* long */
    1, 1, 0,  /* long long */
    1, 1, 1,  /* float */
    1, 1, 1,  /* double */
    1, 1, 1,  /* long double */
    1, 1, 0,  /* T * (pointer) */
    0, 1, 0,  /* struct */
    
    1,  // little_endian
    2,  /* mulops_calls     0 if the hardware implements multiply, divide, and remainder
                            1 if only MUL/DIV are calls, 2 if MUL/DIV and variable shifts are calls */
    0,  // wants_callb      0 if the front-end is responsible for implementing functions that return structs
    0,  // wants_argb       0 if the front-end is responsible for implementing arguments that are structs
    0,  // left_to_right    0 if arguments are evaluated and presented to the back-end right to left
    0,  // wants_dag        0 if the front-end undags all nodes with reference counts exceeding one
    0,  // unsigned_char    0 if char is a signed type
    address,
    blockbeg,
    blockend,
    defaddress,
    defconst,
    defstring,
    defsymbol,
    emit,
    export,
    function,
    gen,
    global,
    import,
    local,
    progbeg,
    progend,
    segment,
    space,
    0, 0, 0, 0, 0, 0, 0,
    {1, rmap,
        blkfetch, blkstore, blkloop,
        _label,
        _rule,
        _nts,
        _kids,
        _string,
        _templates,
        _isinstruction,
        _ntname,
        emit2,
        doarg,
        target,
        clobber,
        info_var,
        var_register,
}
};
static char rcsid[] = "$Id$";
