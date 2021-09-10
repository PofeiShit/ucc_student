#include "ucl.h"
#include "ast.h"
#include "expr.h"

AstExpression FoldConstant(AstExpression expr)
{
	if (expr->op >= OP_BITOR && expr->op <= OP_MOD &&
	    ! (expr->kids[0]->op == OP_CONST && expr->kids[1]->op == OP_CONST))
		return expr;
}