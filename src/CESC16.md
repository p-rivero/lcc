%{
enum { R_ZERO=0, R_SP=1, R_BP=2, R_T0=3, R_T1=4, R_T2=5, R_T3=6, R_A0=7, R_A1=8, R_A2=9, R_A3=10, R_S0=11, R_S1=12, R_S2=13, R_S3=14, R_S4=15};
static char *regnames[] = {"zero", "sp", "bp", "t0", "t1", "t2", "t3", "a0", "a1", "a2", "a3", "s0", "s1", "s2", "s3", "s4"};

// todo: add v0 as temp reg?
static const int INTTMP = (1<<R_T0)|(1<<R_T1)|(1<<R_T2)|(1<<R_T3);
static const int INTVAR = (1<<R_S0)|(1<<R_S1)|(1<<R_S2)|(1<<R_S3)|(1<<R_S4);
static const int INTRET = (1<<R_A0);

static const int ARGREG_AMOUNT = R_A3 - R_A0 + 1;

#include "c.h"
#define NODEPTR_TYPE Node
#define OP_LABEL(p) ((p)->op)
#define LEFT_CHILD(p) ((p)->kids[0])
#define RIGHT_CHILD(p) ((p)->kids[1])
#define STATE_LABEL(p) ((p)->x.state)
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
extern int ckstack(Node, int);
extern int memop(Node);
extern int sametree(Node, Node);
static Symbol intreg[32];
static Symbol fltreg[32];

static Symbol intregw, fltregw;

static int cseg;

// Output of ops.c (arguments: c=1 s=1 i=1 l=1 h=1 f=1 d=1 x=1 p=1)
/* Extra operators:
    - LOADI1=1253
    - LOADU1=1254
    - LOADP1=1255
    - VREGP=711
*/
// Todo: add long support
%}
%start stmt
%term CNSTF1=1041
%term CNSTI1=1045
%term CNSTP1=1047
%term CNSTU1=1046

%term ARGB=41
%term ARGF1=1057
%term ARGI1=1061
%term ARGP1=1063
%term ARGU1=1062

%term ASGNB=57
%term ASGNF1=1073
%term ASGNI1=1077
%term ASGNP1=1079
%term ASGNU1=1078

%term INDIRB=73
%term INDIRF1=1089
%term INDIRI1=1093
%term INDIRP1=1095
%term INDIRU1=1094

%term CVFF1=1137
%term CVFI1=1141

%term CVIF1=1153
%term CVII1=1157
%term CVIU1=1158

%term CVPU1=1174

%term CVUI1=1205
%term CVUP1=1207
%term CVUU1=1206

%term NEGF1=1217
%term NEGI1=1221

%term CALLB=217
%term CALLF1=1233
%term CALLI1=1237
%term CALLP1=1239
%term CALLU1=1238
%term CALLV=216

%term RETF1=1265
%term RETI1=1269
%term RETP1=1271
%term RETU1=1270
%term RETV=248

%term ADDRGP1=1287

%term ADDRFP1=1303

%term ADDRLP1=1319

%term ADDF1=1329
%term ADDI1=1333
%term ADDP1=1335
%term ADDU1=1334

%term SUBF1=1345
%term SUBI1=1349
%term SUBP1=1351
%term SUBU1=1350

%term LSHI1=1365
%term LSHU1=1366

%term MODI1=1381
%term MODU1=1382

%term RSHI1=1397
%term RSHU1=1398

%term BANDI1=1413
%term BANDU1=1414

%term BCOMI1=1429
%term BCOMU1=1430

%term BORI1=1445
%term BORU1=1446

%term BXORI1=1461
%term BXORU1=1462

%term DIVF1=1473
%term DIVI1=1477
%term DIVU1=1478

%term MULF1=1489
%term MULI1=1493
%term MULU1=1494

%term EQF1=1505
%term EQI1=1509
%term EQU1=1510

%term GEF1=1521
%term GEI1=1525
%term GEU1=1526

%term GTF1=1537
%term GTI1=1541
%term GTU1=1542

%term LEF1=1553
%term LEI1=1557
%term LEU1=1558

%term LTF1=1569
%term LTI1=1573
%term LTU1=1574

%term NEF1=1585
%term NEI1=1589
%term NEU1=1590

%term JUMPV=584

%term LABELV=600

%term LOADB=233
%term LOADI1=1253
%term LOADU1=1254
%term LOADP1=1255

%term VREGP=711
%%

reg:  INDIRF1(VREGP)     "# read register\n"
reg:  INDIRI1(VREGP)     "# read register\n"
reg:  INDIRP1(VREGP)     "# read register\n"
reg:  INDIRU1(VREGP)     "# read register\n"

stmt: ASGNF1(VREGP,reg)  "# write register\n"
stmt: ASGNI1(VREGP,reg)  "# write register\n"
stmt: ASGNP1(VREGP,reg)  "# write register\n"
stmt: ASGNU1(VREGP,reg)  "# write register\n"

con: CNSTF1  "%a"
con: CNSTI1  "%a"
con: CNSTP1  "%a"
con: CNSTU1  "%a"

stmt: reg  ""
acon: ADDRGP1  "%a"
acon: con      "%0"
addr_b: acon           "%0"
addr_b: reg            "%0"
addr: addr_b           "%0"
addr: ADDI1(reg,acon)  "%0+%1" 1
addr: ADDP1(reg,acon)  "%0+%1" 1
addr: ADDU1(reg,acon)  "%0+%1" 1
addr: ADDI1(reg,reg)   "%0+%1" 1
addr: ADDP1(reg,reg)   "%0+%1" 1
addr: ADDU1(reg,reg)   "%0+%1" 1
addr: ADDRFP1          "bp+%a" 1
addr: ADDRLP1          "bp+%a" 1

mem: INDIRI1(addr)  "[%0]" 1
mem: INDIRP1(addr)  "[%0]" 1
mem: INDIRU1(addr)  "[%0]" 1
mem_b: INDIRI1(addr_b)  "[%0]" 1
mem_b: INDIRP1(addr_b)  "[%0]" 1
mem_b: INDIRU1(addr_b)  "[%0]" 1

rc:   reg  "%0"
rc:   con  "%0"
rc:   acon "%0"

mrc: mem  "%0"
mrc: rc   "%0"

reg: acon         "\tmov %c, %0\n"  2
reg: ADDRFP1      "\tadd %c, bp, %a\n"  3
reg: ADDRLP1      "\tadd %c, bp, %a\n"  3
reg: mrc          "\tmov %c, %0\n"  2
reg: LOADI1(reg)  "# move\n"  1
reg: LOADU1(reg)  "# move\n"  1
reg: LOADP1(reg)  "# move\n"  1
reg: CNSTF1  "# reg\n"  range(a, 0, 0)
reg: CNSTI1  "# reg\n"  range(a, 0, 0)
reg: CNSTP1  "# reg\n"  range(a, 0, 0)
reg: CNSTU1  "# reg\n"  range(a, 0, 0)

reg: ADDI1(reg,mrc)  "\tadd %c, %0, %1\n"  3
reg: ADDP1(reg,mrc)  "\tadd %c, %0, %1\n"  3
reg: ADDU1(reg,mrc)  "\tadd %c, %0, %1\n"  3
reg: SUBI1(reg,mrc)  "\tsub %c, %0, %1\n"  3
reg: SUBP1(reg,mrc)  "\tsub %c, %0, %1\n"  3
reg: SUBU1(reg,mrc)  "\tsub %c, %0, %1\n"  3
reg: BANDI1(reg,mrc) "\tand %c, %0, %1\n"  3
reg: BORI1(reg,mrc)  "\tor %c, %0, %1\n"   3
reg: BXORI1(reg,mrc) "\txor %c, %0, %1\n"  3
reg: BANDU1(reg,mrc) "\tand %c, %0, %1\n"  3
reg: BORU1(reg,mrc)  "\tor %c, %0, %1\n"   3
reg: BXORU1(reg,mrc) "\txor %c, %0, %1\n"  3

stmt: ASGNI1(addr,ADDI1(mem,reg))   "\tadd %1, %2\n"  memop(a)
stmt: ASGNI1(addr,SUBI1(mem,reg))   "\tsub %1, %2\n"  memop(a)
stmt: ASGNU1(addr,ADDU1(mem,reg))   "\tadd %1, %2\n"  memop(a)
stmt: ASGNU1(addr,SUBU1(mem,reg))   "\tsub %1, %2\n"  memop(a)
stmt: ASGNI1(addr,BANDI1(mem,reg))  "\tand %1, %2\n"  memop(a)
stmt: ASGNI1(addr,BORI1(mem,reg))   "\tor %1, %2\n"   memop(a)
stmt: ASGNI1(addr,BXORI1(mem,reg))  "\txor %1, %2\n"  memop(a)
stmt: ASGNU1(addr,BANDU1(mem,reg))  "\tand %1, %2\n"  memop(a)
stmt: ASGNU1(addr,BORU1(mem,reg))   "\tor %1, %2\n"   memop(a)
stmt: ASGNU1(addr,BXORU1(mem,reg))  "\txor %1, %2\n"  memop(a)
reg: BCOMI1(reg)  "\tnot %c, %0\n"  3
reg: BCOMU1(reg)  "\tnot %c, %0\n"  3
reg: NEGI1(reg)   "\tsub %c, zero, %0\n"  3

con5: CNSTI1  "%a"  range(a, 0, 15)
reg: LSHI1(reg,con5)  "\tsll %c, %0, %1\n"  2
reg: LSHU1(reg,con5)  "\tsll %c, %0, %1\n"  2
reg: RSHI1(reg,con5)  "\tsra %c, %0, %1\n"  2
reg: RSHU1(reg,con5)  "\tsrl %c, %0, %1\n"  2

reg: LSHI1(reg,reg)   "\tsll %c, %0, %1\n"  15
reg: LSHU1(reg,reg)   "\tsll %c, %0, %1\n"  15
reg: RSHI1(reg,reg)   "\tsra %c, %0, %1\n"  15
reg: RSHU1(reg,reg)   "\tsrl %c, %0, %1\n"  15
reg: MULI1(reg,reg)   "\tcall mul\n"  100
reg: MULU1(reg,reg)   "\tcall mul\n"  100
reg: DIVU1(reg,reg)   "\tcall divu\n" 100
reg: MODU1(reg,reg)   "\tcall divu\n" 100
reg: DIVI1(reg,reg)   "\tcall div\n"  100
reg: MODI1(reg,reg)   "\tcall div\n"  100
reg: CVPU1(reg)       "\tmov %c, %0\n"  move(a)
reg: CVUP1(reg)       "\tmov %c, %0\n"  move(a)
reg: CVII1(INDIRI1(addr))  "\tmov %c, [%0]\n"  3
reg: CVUU1(INDIRU1(addr))  "\tmov %c, [%0]\n"  3
reg: CVII1(reg)      "\tmov %c, %0\n"  move(a)
reg: CVIU1(reg)      "\tmov %c, %0\n"  move(a)
reg: CVUI1(reg)      "\tmov %c, %0\n"  move(a)
reg: CVUU1(reg)      "\tmov %c, %0\n"  move(a)

stmt: ASGNI1(addr,reg)  "\tmov [%0], %1\n"  3
stmt: ASGNU1(addr,reg)  "\tmov [%0], %1\n"  3
stmt: ASGNP1(addr,reg)  "\tmov [%0], %1\n"  3
stmt: ARGI1(mrc)  "\tpush %0\n"  3
stmt: ARGU1(mrc)  "\tpush %0\n"  3
stmt: ARGP1(mrc)  "\tpush %0\n"  3
stmt: ASGNB(reg,INDIRB(reg))  "\t ; Todo: copy %a bytes\n"
stmt: ARGB(INDIRB(reg))  "# ARGB\n"
memf: INDIRF1(addr)         "[%0]"
memf: INDIRF1(addr)         "[%0]"
memf: CVFF1(INDIRF1(addr))  "[%0]"
reg: memf  "fld %0\n"  3
stmt: ASGNF1(addr,reg)         "\tfstp [%0]\n"  7
stmt: ASGNF1(addr,CVFF1(reg))  "\tfstp [%0]\n"  7
stmt: ARGF1(reg)  "\tsub sp, sp, 1\n\tfstp [sp]\n"
reg: NEGF1(reg)  "\tfchs\n"
flt: memf  " %0"
flt: reg   "p st(1),st"
reg: ADDF1(reg,flt)  "\tfadd%1\n"
reg: DIVF1(reg,flt)  "\tfdiv%1\n"
reg: MULF1(reg,flt)  "\tfmul%1\n"
reg: SUBF1(reg,flt)  "\tfsub%1\n"
reg: CVFF1(reg)  "\tsub sp, sp, 1\n\tfstp [sp]\n\tfld [sp]\n\tadd sp, sp, 1\n"  12

reg: CVFI1(reg)  "\tcall __ftol\n" 31
reg: CVIF1(reg)  "\tpush %0\n\tfild [sp]\n\tadd sp, sp, 1\n"  12

addrj: ADDRGP1  "%a"
addrj: reg      "%0"
comp:  rc       "%0"
comp:  mem_b    "%0"  1

stmt: LABELV        "%a:\n"
stmt: JUMPV(addrj)  "\tjmp %0\n"  2
stmt: EQI1(reg,comp)  "\tcmp %0, %1\n\tje %a\n"   5
stmt: GEI1(reg,comp)  "\tcmp %0, %1\n\tjge %a\n"  5
stmt: GTI1(reg,comp)  "\tcmp %0, %1\n\tjg %a\n"   5
stmt: LEI1(reg,comp)  "\tcmp %0, %1\n\tjle %a\n"  5
stmt: LTI1(reg,comp)  "\tcmp %0, %1\n\tjl %a\n"   5
stmt: NEI1(reg,comp)  "\tcmp %0, %1\n\tjne %a\n"  5

stmt: EQU1(reg,comp)  "\tcmp %0, %1\n\tje %a\n"   5
stmt: GEU1(reg,comp)  "\tcmp %0, %1\n\tjae %a\n"  5
stmt: GTU1(reg,comp)  "\tcmp %0, %1\n\tja %a\n"   5
stmt: LEU1(reg,comp)  "\tcmp %0, %1\n\tjbe %a\n"  5
stmt: LTU1(reg,comp)  "\tcmp %0, %1\n\tjb %a\n"   5
stmt: NEU1(reg,comp)  "\tcmp %0, %1\n\tjne %a\n"  5
cmpf: memf  " %0"
cmpf: reg   "p"

stmt: EQF1(cmpf,reg)  "\tfcomp%0\n\tfstsw ax\n\tsahf\n\tjp %b\n\tje %a\n%b:\n"
stmt: GEF1(cmpf,reg)  "\tfcomp%0\n\tfstsw ax\n\tsahf\n\tjp %a\n\tjbe %a\n\n"
stmt: GTF1(cmpf,reg)  "\tfcomp%0\n\tfstsw ax\n\tsahf\n\tjp %a\n\tjb %a\n"
stmt: LEF1(cmpf,reg)  "\tfcomp%0\n\tfstsw ax\n\tsahf\n\tjp %a\n\tjae %a\n\n"
stmt: LTF1(cmpf,reg)  "\tfcomp%0\n\tfstsw ax\n\tsahf\n\tjp %a\n\tja %a\n"
stmt: NEF1(cmpf,reg)  "\tfcomp%0\n\tfstsw ax\n\tsahf\n\tjp %a\n\tjne %a\n"
reg:  CALLI1(addrj)   "# call\n"
reg:  CALLU1(addrj)   "# call\n"
reg:  CALLP1(addrj)   "# call\n"
stmt: CALLV(addrj)    "# call\n"
reg:  CALLF1(addrj)   "# call\n"
stmt: CALLF1(addrj)   "\tcall %0\n\tadd sp, sp, %a\nfstp\n"

stmt: RETI1(reg)  "# ret\n"
stmt: RETU1(reg)  "# ret\n"
stmt: RETP1(reg)  "# ret\n"
stmt: RETF1(reg)  "# ret\n"
%%

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
    // Create register bank
    for (i = 0; i < 16; i++)
        intreg[i]  = mkreg(regnames[i], i, 1, IREG);
    
    for (i = 0; i < 8; i++)
        fltreg[i] = mkreg("%d", i, 0, FREG);
    intregw = mkwildcard(intreg);
    fltregw = mkwildcard(fltreg);
    
    tmask[IREG] = INTTMP;
    vmask[IREG] = INTVAR;
    tmask[FREG] = 0xff;
    vmask[FREG] = 0;
    
    // Don't start at any specific bank
    cseg = 0;
}


static Symbol rmap(int opk) {
    switch (optype(opk)) {
    case B: case P: case I: case U:
        return intregw;            
    case F:
        return fltregw;
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


static void target(Node p) {
    assert(p);
    switch (specific(p->op)) {
    case MUL+I: case MUL+U: case DIV+I: case DIV+U:
        setreg(p, intreg[R_A0]);        // Result location
        rtarget(p, 0, intreg[R_A0]);    // Where to store arg
        rtarget(p, 1, intreg[R_A1]);    // Where to store arg
        break;
    
    case MOD+I: case MOD+U:
        setreg(p, intreg[R_A0]);     // Result location
        rtarget(p, 0, intreg[R_A0]); // Where to store arg
        rtarget(p, 1, intreg[R_A1]); // Where to store arg
        break;
        
    case CNST+I: case CNST+U: case CNST+P:
        // Use zero register for constants that are 0
        if (range(p, 0, 0) == 0) {
            setreg(p, intreg[R_ZERO]); 
            p->x.registered = 1;
        }
        break;
        
    case ASGN+B:
        rtarget(p, 0, intreg[R_T1]); // Where to store destination address
        rtarget(p->kids[1], 0, intreg[R_T0]); // Where to store source address
        break;
        
    case ARG+B:
        rtarget(p->kids[0], 0, intreg[R_T0]); // Where to store source address
        break;
        
    case CVF+I:
        setreg(p, intreg[R_A0]); // Result location
        break;
        
    case CALL+I: case CALL+U: case CALL+P: case CALL+V:
        rtarget(p, 0, intreg[R_T3]);
        setreg(p, intreg[R_A0]); // Result location
        break;
        
    case RET+I: case RET+U: case RET+P:
        rtarget(p, 0, intreg[R_A0]); // Where to store ret value
        break;
    }
}

static void clobber(Node p) {
    static int nstack = 0;

    assert(p);
    nstack = ckstack(p, nstack);
    switch (specific(p->op)) {        
    case ASGN+B: case ARG+B:
        // Which regs to free
        // Todo: if block copy requires more regs, spill them here
        spill((1<<R_T0)|(1<<R_T1), IREG, p);
        break;
            
    case CALL+F:
        spill(INTTMP|INTRET, IREG, p);
        break;
        
    case CALL+I: case CALL+U: case CALL+P:
        spill(INTTMP, IREG, p);
        break;
        
    case CALL+V:
        spill(INTTMP|INTRET, IREG, p);
        break;
        
    case MUL+I: case MUL+U: case DIV+I: case DIV+U: case MOD+I: case MOD+U:
        // Todo: spill only the arg regs?
        spill(INTTMP, IREG, p);
        break;
    }
}


#define isfp(p) (optype((p)->op)==F)
#define preg(f) ((f)[getregnum(p->x.kids[0])]->x.name)


static void emit2(Node p) {
    int op = specific(p->op);

    if (generic(op) == CVI || generic(op) == CVU || generic(op) == LOAD) {
        char *dst = intreg[getregnum(p)]->x.name;
        char *src = preg(intreg);
        assert(opsize(p->op) <= opsize(p->x.kids[0]->op));
        if (dst != src)
            print("\tmov %s, %s\n", dst, src);
    }
    else if (op == ARG+B) {
        int struct_size = p->syms[0]->u.c.v.i;
        print("\tsub sp, %d\n\tmov t1, sp\n", struct_size);
        print("\t; Todo: copy %d words. Source at t0, destination at t1\n", struct_size);
    }
    else if (op == CALL+I || op == CALL+U  || op == CALL+P || op == CALL+V) {
        print("\tcall %s\n", p->kids[0]->syms[0]->x.name);
        char *arg_sz = p->syms[0]->x.name;
        if (strcmp(arg_sz, "0"))   // arg_sz != "0"
            print("\tadd sp, sp, %s\n", arg_sz);
    }
    // Todo: emit ARG operators (op == ARG+I || op == ARG+U || op == ARG+P)
}

// static Symbol argreg(argno, offset, ty) int argno, offset, ty; {
//     // Select an argument register, return null if the arg is stored in stack
//     if (ty == F || ty == D) assert(0); // Floats not supported
    
//     if (offset >= ARGREG_AMOUNT) return NULL;
//     else return ireg[R_A0 + offset];
// }
static void doarg(Node p) {
    assert(p && p->syms[0]);
    mkactual(1, p->syms[0]->u.c.v.i);
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
    if (isfloat(p->type))
        p->sclass = AUTO;
    if (askregvar(p, (*IR->x.rmap)(ttob(p->type))) == 0) {
        assert(p->sclass == AUTO);
        offset = roundup(offset + p->type->size, p->type->align);
        p->x.offset = -offset;
        p->x.name = stringd(-offset);
    }
}


static void function(Symbol f, Symbol caller[], Symbol callee[], int n) {
    int i;
    
    segment(CODE);
    print("\n%s:\n", f->x.name);
    print("\tpush bp\n");
    print("\tmov bp, sp\n");
    usedmask[0] = usedmask[1] = 0;
    freemask[0] = freemask[1] = ~(unsigned)0;
    offset = 2; // Skip stored bp and ret value
    
    for (i = 0; callee[i]; i++) {
        Symbol p = callee[i];
        Symbol q = caller[i];
        assert(q);
        p->x.offset = q->x.offset = offset;
        p->x.name = q->x.name = stringf("%d", p->x.offset);
        p->sclass = q->sclass = AUTO;
        offset += q->type->size; // Don't round up
    }
    
    assert(caller[i] == 0);
    offset = maxoffset = 0;
    
    gencode(caller, callee);
    
    framesize = maxoffset; // Don't round up the frame size
    usedmask[IREG] &= INTVAR; // Only save the safe registers
    
    if (framesize > 0) print("\tsub sp, sp, %d\n", framesize);
    
    // Store safe regs
    for (i = R_S0; i <= R_S4; i++) {
        if (usedmask[IREG] & (1<<i)) {
            print("\tpush %s\n", regnames[i]);
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
    print("\tpop bp\n");
    print("\tret\n");
}


static void defsymbol(Symbol p) {
    if (p->scope >= LOCAL && p->sclass == STATIC) {
        p->x.name = stringf(".L%d", genlabel(1));
    }
    else if (p->generated) {
        p->x.name = stringf(".L%s", p->name);
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
    if (suffix == I && size == 1)
        print("#d16 %d\n",   v.u);
    else if (suffix == U && size == 1)
        print("#d16 0x%x\n", (unsigned)((unsigned char)v.u));
    else if (suffix == P && size == 1)
        print("#d16 0x%x\n", (unsigned long long)v.p);
    else if (suffix == F && size == 1) {
        float f = v.d;
        print("#d16 0x%x\n", *(unsigned *)&f);
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
        print("; extern %s\n", p->x.name);
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


// Not byte-oriented: integer size is 1 word
Interface CESC16IR = {
    // Type metrics: Size, Align and outofline flag
    // If outofline=1, constants cannot appear in dags
    1, 1, 0,  /* char */
    1, 1, 0,  /* short */
    1, 1, 0,  /* int */
    1, 1, 0,  /* long */    // Todo: Add support for long
    1, 1, 0,  /* long long */
    1, 1, 1,  /* float */
    1, 1, 1,  /* double */
    1, 1, 1,  /* long double */
    1, 1, 0,  /* T * (pointer) */
    0, 1, 0,  /* struct */
    1,  // little_endian
    1,  // mulops_calls     0 if the hardware implements multiply, divide, and remainder
    0,  // wants_callb      0 if the front-end is responsible for implementing functions that return structs
    0,  // wants_argb       0 if the front-end is responsible for implementing arguments that are structs
    0,  // left_to_right    0 if arguments are evaluated and presented to the back-end right to left
    0,  // wants_dag        0 if the front-end undags all nodes with reference counts exceeding 
    0,        /* unsigned_char */
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
}
};
static char rcsid[] = "$Id$";
