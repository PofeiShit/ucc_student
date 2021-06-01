#include "ucl.h"
#include "ast.h"
#include "expr.h"
#include "decl.h"

static AstExpression CheckPrimaryExpression(AstExpression expr)
{
	Symbol p;
	if (expr->op == OP_STR) {
		//expr->op = OP_ID;
		expr->val.p = AddString(expr->val.p);
		return expr;
	}
}

static AstExpression CheckFunctionCall(AstExpression expr)
{
	if (expr->kids[0]->op == OP_ID) {
		expr->kids[0]->val.p = AddFunction(expr->kids[0]->val.p);
		// fast show hello world
		//expr->kids[0]->val.p = AddFunction("puts");
	}
	CheckExpression(expr->kids[1]);
	return expr;
}

static AstExpression CheckPostfixExpression(AstExpression expr)
{
	switch(expr->op) {
		case OP_CALL:
			return CheckFunctionCall(expr);
		default:
			//assert(0);
			;
	}
}

static AstExpression (*ExprCheckers[])(AstExpression) = 
{
#define OPINFO(op, prec, name, func, opcode) Check##func##Expression,
#include "opinfo.h"
#undef OPINFO
};

AstExpression CheckExpression(AstExpression expr)
{
	return (*ExprCheckers[expr->op])(expr);
}
