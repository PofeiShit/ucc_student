#include "ucl.h"
#include "ast.h"
#include "expr.h"
#include "decl.h"

static AstExpression CheckPrimaryExpression(AstExpression expr)
{
	Symbol p;
	if (expr->op == OP_CONST) {
		return expr;
	}
	if (expr->op == OP_STR) {
		//expr->op = OP_ID;
		expr->val.p = AddString(expr->ty, expr->val.p);
		return expr;
	}
}

static AstExpression CheckFunctionCall(AstExpression expr)
{
	Type ty;
	if (expr->kids[0]->op == OP_ID) {
		expr->kids[0]->ty = DefaultFunctionType;
		expr->kids[0]->val.p = AddFunction(expr->kids[0]->val.p);
	}
	ty = expr->kids[0]->ty;

	if (expr->kids[1] != NULL)
		CheckExpression(expr->kids[1]);
	expr->ty = ty->bty;
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
