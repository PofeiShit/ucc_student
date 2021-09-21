#ifndef TEMPLATE
#error "You must define TEMPLATE macro before include this file"
#endif



TEMPLATE(X86_BORI4,     "orl %2, %0")
TEMPLATE(X86_BORU4,     "orl %2, %0")
TEMPLATE(X86_BORF4,     NULL)
TEMPLATE(X86_BORF8,     NULL)

TEMPLATE(X86_BXORI4,    "xorl %2, %0")
TEMPLATE(X86_BXORU4,    "xorl %2, %0")
TEMPLATE(X86_BXORF4,    NULL)
TEMPLATE(X86_BXORF8,    NULL)

TEMPLATE(X86_BANDI4,    "andl %2, %0")
TEMPLATE(X86_BANDU4,    "andl %2, %0")
TEMPLATE(X86_BANDF4,    NULL)
TEMPLATE(X86_BANDF8,    NULL)

TEMPLATE(X86_LSHI4,    "shll %2, %0")
TEMPLATE(X86_LSHU4,    "shll %2, %0")
TEMPLATE(X86_LSHF4,    NULL)
TEMPLATE(X86_LSHF8,    NULL)

TEMPLATE(X86_RSHI4,    "sarl %2, %0")
TEMPLATE(X86_RSHU4,    "shrl %2, %0")
TEMPLATE(X86_RSHF4,    NULL)
TEMPLATE(X86_RSHF8,    NULL)

TEMPLATE(X86_ADDI4,    "addl %2, %0")
TEMPLATE(X86_ADDU4,    "addl %2, %0")
TEMPLATE(X86_ADDF4,    "fadds %2")
TEMPLATE(X86_ADDF8,    "faddl %2")

TEMPLATE(X86_SUBI4,    "subl %2, %0")
TEMPLATE(X86_SUBU4,    "subl %2, %0")
TEMPLATE(X86_SUBF4,    "fsubs %2")
TEMPLATE(X86_SUBF8,    "fsubl %2")

TEMPLATE(X86_MULI4,    "imull %2, %0")
TEMPLATE(X86_MULU4,    "mull %2")
TEMPLATE(X86_MULF4,    "fmuls %2")
TEMPLATE(X86_MULF8,    "fmull %2")

TEMPLATE(X86_DIVI4,    "cdq;idivl %2")
TEMPLATE(X86_DIVU4,    "movl $0, %%edx;divl %2")
TEMPLATE(X86_DIVF4,    "fdivs %2")
TEMPLATE(X86_DIVF8,    "fdivl %2")

TEMPLATE(X86_MODI4,    "cdq;idivl %2")
TEMPLATE(X86_MODU4,    "movl $0, %%edx; divl %2")
TEMPLATE(X86_MODF4,    NULL)
TEMPLATE(X86_MODF8,    NULL)

TEMPLATE(X86_JZI4,     "cmpl $0, %1;je %0")
TEMPLATE(X86_JZU4,     "cmpl $0, %1;je %0")
TEMPLATE(X86_JZF4,     "fldz;fucompp;fnstsw %%ax;test $0x44, %%ah;jnp %0")
TEMPLATE(X86_JZF8,     "fldz;fucompp;fnstsw %%ax;test $0x44, %%ah;jnp %0")

TEMPLATE(X86_JNZI4,    "cmpl $0, %1;jne %0")
TEMPLATE(X86_JNZU4,    "cmpl $0, %1;jne %0")
TEMPLATE(X86_JNZF4,    "fldz;fucompp;fnstsw %%ax;test $0x44, %%ah;jp %0")
TEMPLATE(X86_JNZF8,    "fldz;fucompp;fnstsw %%ax;test $0x44, %%ah;jp %0")

TEMPLATE(X86_ADDR,     "leal %1, %0")

TEMPLATE(X86_PROLOGUE, "pushl %%ebp;pushl %%ebx;pushl %%esi;pushl %%edi;movl %%esp, %%ebp")
TEMPLATE(X86_PUSH,     "pushl %0")
TEMPLATE(X86_EXPANDF,  "subl %0, %%esp")
TEMPLATE(X86_CALL,     "call %1")
TEMPLATE(X86_REDUCEF,  "addl %0, %%esp")
TEMPLATE(X86_EPILOGUE, "movl %%ebp, %%esp;popl %%edi;popl %%esi;popl %%ebx;popl %%ebp;ret")
TEMPLATE(X86_MOVI1,   "movb %1, %0")
TEMPLATE(X86_MOVI4,   "movl %1, %0")
TEMPLATE(X86_JMP, "jmp %0")
