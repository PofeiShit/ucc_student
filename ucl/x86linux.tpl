#ifndef TEMPLATE
#error "You must define TEMPLATE macro before include this file"
#endif


TEMPLATE(X86_ADDR,     "leal %1, %0")

TEMPLATE(X86_PROLOGUE, "pushl %%ebp;pushl %%ebx;pushl %%esi;pushl %%edi;movl %%esp, %%ebp")
TEMPLATE(X86_PUSH,     "pushl %0")
TEMPLATE(X86_EXPANDF,  "subl %0, %%esp")
TEMPLATE(X86_CALL,     "call %1")
TEMPLATE(X86_REDUCEF,  "addl %0, %%esp")
TEMPLATE(X86_EPILOGUE, "movl %%ebp, %%esp;popl %%edi;popl %%esi;popl %%ebx;popl %%ebp;ret")
