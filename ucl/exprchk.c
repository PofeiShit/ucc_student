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
	p = LookupID(expr->val.p);
	{
		expr->ty = p->ty;
		expr->val.p = p;
	}
	return expr;
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
	
	// f(void) 
	if (fty->sig->hasProto && parLen == 0)
	{
		*argFull = 1;
		return arg;
	}
	// f(int, int, int)  and check the last one parameter			
	if (argNo == parLen && ! fty->sig->hasEllipsis)
		*argFull = 1;

	// TODO:delete old style
	if (!fty->sig->hasProto) 
	{
		// arg = PromoteArgument(arg);
		*argFull = 0;
		return arg;
	} else if (argNo <= parLen) {
		param = GET_ITEM(fty->sig->params, argNo - 1);
		// 
		if (param->ty->categ < INT)
			arg = Cast(T(INT), arg);
		else
			arg = Cast(param->ty, arg);
		return arg;
	}
}

static AstExpression CheckFunctionCall(AstExpression expr)
{
	AstExpression arg;
	Type ty;
	AstNode *tail;
	int argNo, argFull;
	
	if (expr->kids[0]->op == OP_ID && LookupID(expr->kids[0]->val.p) == NULL) {
		expr->kids[0]->ty = DefaultFunctionType;
		expr->kids[0]->val.p = AddFunction(expr->kids[0]->val.p, DefaultFunctionType);
	} else {
		expr->kids[0] = CheckExpression(expr->kids[0]);
	}
	expr->kids[0] = Adjust(expr->kids[0], 1);
	ty = expr->kids[0]->ty; // ty is PointerType
	if (!(IsPtrType(ty) && IsFunctionType(ty->bty))) {
		printf("ty is ptr and base ty is not function\n");
	}
	ty = ty->bty; // ty is FunctionType

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
/**
 Syntax
 
		   assignment-expression:
				   conditional-expression
				   unary-expression assignment-operator assignment-expression
 
		   assignment-operator: one of
				   =  *=  /=  %=  +=  -=  <<=  >>=	&=	^=	|=s

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
*/
static AstExpression CheckAssignmentExpression(AstExpression expr)
{
	Type ty;
	expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 0);
	expr->kids[1] = Adjust(CheckExpression(expr->kids[1]), 1);

	// we have use CanModify() to test whether left operand is modifiable.
	ty = expr->kids[0]->ty;
	expr->kids[1] = Cast(ty, expr->kids[1]);	
	expr->ty = ty;	
	return expr;
}
static AstExpression CheckCommaExpression(AstExpression expr)
{
	expr->kids[0] = Adjust(CheckExpression(expr->kids[0]), 1);
	expr->kids[0] = Adjust(CheckExpression(expr->kids[1]), 1);

	expr->ty = expr->kids[1]->ty;
	return expr;
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
