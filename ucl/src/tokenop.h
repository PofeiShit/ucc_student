#ifndef TOKENOP
#error "You must define TOKENOP macro before include this file"
#endif
//      Token               binary op       unary op
TOKENOP(TK_ASSIGN,        OP_ASSIGN,        OP_NONE)
TOKENOP(TK_BITOR_ASSIGN,  OP_BITOR_ASSIGN,  OP_NONE)
TOKENOP(TK_BITXOR_ASSIGN, OP_BITXOR_ASSIGN, OP_NONE)
TOKENOP(TK_BITAND_ASSIGN, OP_BITAND_ASSIGN, OP_NONE)
TOKENOP(TK_LSHIFT_ASSIGN, OP_LSHIFT_ASSIGN, OP_NONE)
TOKENOP(TK_RSHIFT_ASSIGN, OP_RSHIFT_ASSIGN, OP_NONE)
TOKENOP(TK_ADD_ASSIGN,    OP_ADD_ASSIGN,    OP_NONE)
TOKENOP(TK_SUB_ASSIGN,    OP_SUB_ASSIGN,    OP_NONE)
TOKENOP(TK_MUL_ASSIGN,    OP_MUL_ASSIGN,    OP_NONE)
TOKENOP(TK_DIV_ASSIGN,    OP_DIV_ASSIGN,    OP_NONE)
TOKENOP(TK_MOD_ASSIGN,    OP_MOD_ASSIGN,    OP_NONE)
