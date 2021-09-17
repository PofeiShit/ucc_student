#include "ucl.h"
#include "ast.h"
#include "expr.h"
#include "gen.h"

/**
 * Translates a primary expression.
 */
static Symbol TranslatePrimaryExpression(AstExpression expr)
{
	if (expr->op == OP_CONST)
		return AddConstant(expr->ty, expr->val);
	/// if the expression is adjusted from an array or a function,
	/// returns the address of the symbol for this identifier
	if (expr->op == OP_STR){
		//#if  1	// added
		//assert(expr->op != OP_STR);
		//#endif
		return AddressOf((Symbol)expr->val.p);
	}

	return (Symbol)expr->val.p;
}

/**
	 postfix-expression:
			 primary-expression
			 postfix-expression (  argument-expression-list<opt> ) 
			 
	expe->op	is 	OP_CALL
*/
static Symbol TranslateFunctionCall(AstExpression expr)
{
	AstExpression arg;
	Symbol faddr, recv;
	ILArg ilarg;
	Vector args = CreateVector(4);
	/**	
		Here, we want to use function name f as function call?
		Function name can be used as function address,
			see TranslatePrimaryExpression(AstExpression expr).
	 */
	// expr->kids[0]->isfunc = 0;
	faddr = TranslateExpression(expr->kids[0]);
	arg = expr->kids[1];
	
	while (arg)
	{
		ALLOC(ilarg);
		// After TranslateExpression(arg), sym->ty may be different from art->ty.
		// See AddressOf() and the following comments for detail.
		ilarg->sym = TranslateExpression(arg);
		ilarg->ty = arg->ty;
		INSERT_ITEM(args, ilarg);
		arg = (AstExpression)arg->next;
	}
	
	recv = NULL;
			
	if (expr->ty->categ != VOID)
	{
		recv = CreateTemp(expr->ty);		
	}
	GenerateFunctionCall(expr->ty, recv, faddr, args);
	return recv;
}
static Symbol TranslatePostfixExpression(AstExpression expr)
{
	switch (expr->op)
	{
	case OP_CALL:
		return TranslateFunctionCall(expr);
	default:
		//assert(0);
		;
		return NULL;
	}
}
static Symbol TranslateBinaryExpression(AstExpression expr)
{
	Symbol src1, src2;
	src1 = TranslateExpression(expr->kids[0]);
	src2 = TranslateExpression(expr->kids[1]);
	return Simplify(expr->ty, OPMap[expr->op], src1, src2);
}

static Symbol TranslateAssignmentExpression(AstExpression expr)
{
	Symbol dst, src;
	dst = TranslateExpression(expr->kids[0]);
	src = TranslateExpression(expr->kids[1]);
	GenerateMove(expr->ty, dst, src);
	return dst;
}
static Symbol TranslateCommaExpression(AstExpression expr)
{
	TranslateExpression(expr->kids[0]);
	return TranslateExpression(expr->kids[1]);
}

static Symbol TranslateErrorExpression(AstExpression expr)
{
	return NULL;
}
static Symbol (* ExprTrans[])(AstExpression) = 
{
#define OPINFO(op, prec, name, func, opcode) Translate##func##Expression,
#include "opinfo.h"
#undef OPINFO
};
Symbol TranslateExpression(AstExpression expr)
{
	return (* ExprTrans[expr->op])(expr);
}