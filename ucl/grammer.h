#ifndef __GRAMMER_H_
#define __GRAMMER_H_
//  tokens that may be first token in a declaration
#define FIRST_DECLARATION   \
    TK_AUTO, TK_EXTERN, TK_STATIC, \
    TK_CHAR, TK_INT,  \
    TK_STRUCT, TK_VOID, TK_ID
// fisrt token of an expression
#define FIRST_EXPRESSION  \
    TK_ID, TK_INTCONST,  \
    TK_STRING, TK_LPAREN
// first token of statement
#define FIRST_STATEMENT                                                                   \
    TK_LBRACE, TK_RETURN,  TK_SEMICOLON, FIRST_EXPRESSION

#endif


