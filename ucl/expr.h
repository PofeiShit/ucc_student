#ifndef __EXPR_H_
#define __EXPR_H_

enum OP
{
#define OPINFO(op, prec, name, func, opcode) op,
#include "opinfo.h"
#undef OPINFO
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
AstExpression CheckExpression(AstExpression expr);
Symbol TranslateExpression(AstExpression expr);
#endif
