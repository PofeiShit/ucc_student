#ifndef OPINFO
#error "You must define OPINFO macro before include this file"
#endif
/**
	opinfo here is used by AST
 */

OPINFO(OP_COMMA,         1,    ",",      Comma,          NOP)
OPINFO(OP_ASSIGN,        2,    "=",      Assignment,     NOP)
OPINFO(OP_BITOR_ASSIGN,  2,    "|=",     Assignment,     NOP)
OPINFO(OP_BITXOR_ASSIGN, 2,    "^=",     Assignment,     NOP)
OPINFO(OP_BITAND_ASSIGN, 2,    "&=",     Assignment,     NOP)
OPINFO(OP_LSHIFT_ASSIGN, 2,    "<<=",    Assignment,     NOP)
OPINFO(OP_RSHIFT_ASSIGN, 2,    ">>=",    Assignment,     NOP)
OPINFO(OP_ADD_ASSIGN,    2,    "+=",     Assignment,     NOP)
OPINFO(OP_SUB_ASSIGN,    2,    "-=",     Assignment,     NOP)
OPINFO(OP_MUL_ASSIGN,    2,    "*=",     Assignment,     NOP)
OPINFO(OP_DIV_ASSIGN,    2,    "/=",     Assignment,     NOP)
OPINFO(OP_MOD_ASSIGN,    2,    "%=",     Assignment,     NOP)
//

OPINFO(OP_BITOR,         6,    "|",      Binary,         BOR)
OPINFO(OP_BITXOR,        7,    "^",      Binary,         BXOR)
OPINFO(OP_BITAND,        8,    "&",      Binary,         BAND)
OPINFO(OP_LSHIFT,        11,   "<<",     Binary,         LSH)
OPINFO(OP_RSHIFT,        11,   ">>",     Binary,         RSH)
OPINFO(OP_ADD,           12,   "+",      Binary,         ADD)
OPINFO(OP_SUB,           12,   "-",      Binary,         SUB)
OPINFO(OP_MUL,           13,   "*",      Binary,         MUL)
OPINFO(OP_DIV,           13,   "/",      Binary,         DIV)
OPINFO(OP_MOD,           13,   "%",      Binary,         MOD)
// 

OPINFO(OP_CALL,          15,   "call",   Postfix,        NOP)
OPINFO(OP_ID,            16,   "id",     Primary,        NOP)
OPINFO(OP_CONST,         16,   "const",  Primary,        NOP)
OPINFO(OP_STR,           16,   "str",    Primary,        NOP)

OPINFO(OP_NONE,          17,   "nop",    Error,          NOP)