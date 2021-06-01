#ifndef OPINFO
#error "You must define OPINFO macro before include this file"
#endif
/**
	opinfo here is used by AST
 */


//

// 

OPINFO(OP_CALL,          15,   "call",   Postfix,        NOP)
OPINFO(OP_ID,            16,   "id",     Primary,        NOP)
OPINFO(OP_STR,           16,   "str",    Primary,        NOP)

