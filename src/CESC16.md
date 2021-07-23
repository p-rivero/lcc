%{
#include "c.h"
#define NODEPTR_TYPE Node
#define OP_LABEL(p) ((p)->op)
#define LEFT_CHILD(p) ((p)->kids[0])
#define RIGHT_CHILD(p) ((p)->kids[1])
#define STATE_LABEL(p) ((p)->x.state)
static int memop(Node);

// Output of ops.c (arguments: c=1 s=1 i=1 l=1 h=1 f=1 d=1 x=1 p=1)
/* Extra operators:
    - LOADI1=1253
    - LOADU1=1254
    - LOADP1=1255
    - VREGP=711
    - ASMV=616
*/
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
%term ASMV=616

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
con4: CNSTI1  "%a"  range(a, 0, 15)

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

reg: ADDRFP1      "\tadd %c, bp, %a\n"  3
reg: ADDRLP1      "\tadd %c, bp, %a\n"  3
reg: mrc          "\tmov %c, %0\n"  2
reg: LOADI1(reg)  "# move\n"  move(a)
reg: LOADU1(reg)  "# move\n"  move(a)
reg: LOADP1(reg)  "# move\n"  move(a)
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

rc4: reg "%0"
rc4: con4 "%0"
stmt: ASGNI1(addr,rc4)  "\tmov [%0], %1\n"  3
stmt: ASGNU1(addr,rc4)  "\tmov [%0], %1\n"  3
stmt: ASGNP1(addr,rc4)  "\tmov [%0], %1\n"  3
stmt: ASGNI1(addr,ADDI1(mem,rc4))   "\tadd %1, %2\n"  memop(a)
stmt: ASGNI1(addr,SUBI1(mem,rc4))   "\tsub %1, %2\n"  memop(a)
stmt: ASGNU1(addr,ADDU1(mem,rc4))   "\tadd %1, %2\n"  memop(a)
stmt: ASGNU1(addr,SUBU1(mem,rc4))   "\tsub %1, %2\n"  memop(a)
stmt: ASGNI1(addr,BANDI1(mem,rc4))  "\tand %1, %2\n"  memop(a)
stmt: ASGNI1(addr,BORI1(mem,rc4))   "\tor %1, %2\n"   memop(a)
stmt: ASGNI1(addr,BXORI1(mem,rc4))  "\txor %1, %2\n"  memop(a)
stmt: ASGNU1(addr,BANDU1(mem,rc4))  "\tand %1, %2\n"  memop(a)
stmt: ASGNU1(addr,BORU1(mem,rc4))   "\tor %1, %2\n"   memop(a)
stmt: ASGNU1(addr,BXORU1(mem,rc4))  "\txor %1, %2\n"  memop(a)
reg: BCOMI1(reg)  "\tnot %c, %0\n"  3
reg: BCOMU1(reg)  "\tnot %c, %0\n"  3
reg: NEGI1(reg)   "\tsub %c, zero, %0\n"  3

reg: LSHI1(reg,con4)  "\tsll %c, %0, %1\n"  2
reg: LSHU1(reg,con4)  "\tsll %c, %0, %1\n"  2
reg: RSHI1(reg,con4)  "\tsra %c, %0, %1\n"  2
reg: RSHU1(reg,con4)  "\tsrl %c, %0, %1\n"  2

reg: LSHI1(reg,reg)   "\tcall var_sll\n"  20
reg: LSHU1(reg,reg)   "\tcall var_sll\n"  20
reg: RSHI1(reg,reg)   "\tcall var_sra\n"  20
reg: RSHU1(reg,reg)   "\tcall var_srl\n"  20
reg: MULI1(reg,reg)   "\tcall mul\n"  100
reg: MULU1(reg,reg)   "\tcall mul\n"  100
reg: DIVU1(reg,reg)   "\tcall divu\n" 100
reg: MODU1(reg,reg)   "\tcall modu\n" 100
reg: DIVI1(reg,reg)   "\tcall div\n"  100
reg: MODI1(reg,reg)   "\tcall mod\n"  100
reg: CVPU1(reg)       "\tmov %c, %0\n"  move(a)
reg: CVUP1(reg)       "\tmov %c, %0\n"  move(a)
reg: CVII1(INDIRI1(addr))  "\tmov %c, [%0]\n"  3
reg: CVUU1(INDIRU1(addr))  "\tmov %c, [%0]\n"  3
reg: CVII1(reg)      "\tmov %c, %0\n"  move(a)
reg: CVIU1(reg)      "\tmov %c, %0\n"  move(a)
reg: CVUI1(reg)      "\tmov %c, %0\n"  move(a)
reg: CVUU1(reg)      "\tmov %c, %0\n"  move(a)

stmt: ARGI1(rc)  "# arg\n"  3
stmt: ARGU1(rc)  "# arg\n"  3
stmt: ARGP1(rc)  "# arg\n"  3
stmt: ASGNB(reg,INDIRB(reg)) "# asgnb\n" 10
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
con0:  CNSTI1   "%a"  range(a, 0, 0)

stmt: LABELV        "%a:\n"
stmt: ASMV          "\t%a\n"
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

stmt: EQI1(mrc,con0)  "\ttest %0\n\tjz %a\n"   4
stmt: NEI1(mrc,con0)  "\ttest %0\n\tjnz %a\n"  4
stmt: EQU1(mrc,con0)  "\ttest %0\n\tjz %a\n"   4
stmt: NEU1(mrc,con0)  "\ttest %0\n\tjnz %a\n"  4


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

#include "CESC16.c"
