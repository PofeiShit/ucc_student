#include "ucl.h"
#include "ast.h"
#include "expr.h"

AstExpression FoldConstant(AstExpression expr)
{
	if (expr->op >= OP_BITOR && expr->op <= OP_MOD &&
	    ! (expr->kids[0]->op == OP_CONST && expr->kids[1]->op == OP_CONST))
		return expr;
}

AstExpression Constant(Type ty, union value val)
{
	AstExpression expr;
	CREATE_AST_NODE(expr, Expression);
	expr->op = OP_CONST;
	expr->ty = ty;
	expr->val = val;
	return expr;
}