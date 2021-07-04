#include "ucl.h"
#include "ast.h"
#include "expr.h"
#include "decl.h"

AstExpression Cast(Type ty, AstExpression expr)
{
	int scode = TypeCode(expr->ty);
	int dcode = TypeCode(ty);
	
	expr->ty = ty;
	return expr;
}

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

static AstExpression PromoteArgument(AstExpression arg)
{
	Type ty = Promote(arg->ty);
	return Cast(ty, arg);
}

AstExpression Adjust(AstExpression expr, int rvalue)
{
	if (expr->ty->categ == FUNCTION) {
		expr->ty = PointerTo(expr->ty);
	} else if (expr->ty->categ == ARRAY)
		expr->ty = PointerTo(expr->ty->bty);
	return expr;
}

static AstExpression CheckArgument(FunctionType fty, AstExpression arg, int argNo, int *argFull)
{
	Parameter param;

	int parLen = LEN(fty->sig->params);

	arg = Adjust(CheckExpression(arg), 1);

	if (fty->sig->hasProto && parLen == 0)
	{
		*argFull = 1;
		return arg;
	}

	if (argNo == parLen && ! fty->sig->hasEllipsis)
		*argFull = 1;

	if (!fty->sig->hasProto) 
	{
		arg = PromoteArgument(arg);
		*argFull = 0;
		return arg;
	}
}

static AstExpression CheckFunctionCall(AstExpression expr)
{
	AstExpression arg;
	Type ty;
	AstNode *tail;
	int argNo, argFull;
	if (expr->kids[0]->op == OP_ID) {
		expr->kids[0]->ty = DefaultFunctionType;
		expr->kids[0]->val.p = AddFunction(expr->kids[0]->val.p, DefaultFunctionType);
	}
	expr->kids[0] = Adjust(expr->kids[0], 1);
	ty = expr->kids[0]->ty;
	ty = ty->bty;	

	tail = (AstNode *)&expr->kids[1];
	arg = expr->kids[1];
	argNo = 1;
	argFull = 0;
	while (arg != NULL && !argFull)
	{
		*tail = (AstNode)CheckArgument((FunctionType)ty, arg, argNo, &argFull);
		tail = &(*tail)->next;
		arg = (AstExpression)arg->next;
		argNo++;
	}
	while (arg != NULL) {
		CheckExpression(expr);
		arg = (AstExpression)arg->next;
	}
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
