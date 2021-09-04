#ifndef __EXPR_H_
#define __EXPR_H_

enum OP
{
#define OPINFO(op, prec, name, func, opcode) op,
#include "opinfo.h"
#undef OPINFO
};
/**
 Some tokens represent both unary and binary operator
e.g. 	*   +
 */
struct tokenOp
{
	// token used as binary operator
	int bop  : 16;
	// token used as unary operator
	int uop  : 16;
};
struct astExpression
{
	AST_NODE_COMMON
	Type ty;
	int op : 16;
	// int isarray : 1;
	// int isfunc  : 1;
	// int lvalue  : 1;
	// int bitfld  : 1;
	// int inreg   : 1;
	// int unused  : 11;
	struct astExpression *kids[2];
	union value val;
};
// test whether it is a binary operator
#define IsBinaryOP(tok) (tok >= TK_EQUAL && tok <= TK_LESS_EQ)
// token used as binary operator
#define	BINARY_OP       TokenOps[CurrentToken - TK_ASSIGN].bop
AstExpression FoldConstant(AstExpression expr);
AstExpression CheckExpression(AstExpression expr);
AstExpression Cast(Type ty, AstExpression expr);
AstExpression Adjust(AstExpression expr, int rvalue);
Symbol TranslateExpression(AstExpression expr);
#endif
