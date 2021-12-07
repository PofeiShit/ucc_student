#include "ucl.h"
#include "ast.h"
#include "expr.h"

#define EXECUTE_BOP(op)													\
	if (tcode == I4) val.i[0] = expr1->val.i[0] op expr2->val.i[0];         

AstExpression FoldCast(Type ty, AstExpression expr)
{
	int scode = TypeCode(expr->ty);
	int dcode = TypeCode(ty);
	expr->ty = ty;
	return expr;
}
AstExpression FoldConstant(AstExpression expr)
{
	int tcode;
	union value val;
	AstExpression expr1, expr2;
	val.i[1] = val.i[0] = 0;
	tcode = TypeCode(expr->kids[0]->ty);
	expr1 = expr->kids[0];
	expr2 = expr->kids[1];

	if (expr->op >= OP_OR && expr->op <= OP_MOD &&
	    ! (expr->kids[0]->op == OP_CONST && expr->kids[1]->op == OP_CONST))
		return expr;

	if (expr->op >= OP_POS && expr->op <= OP_NOT && expr->kids[0]->op != OP_CONST)
		return expr;
	val.i[1] = val.i[0] = 0;
	tcode = TypeCode(expr->kids[0]->ty);
	expr1 = expr->kids[0];
	expr2 = expr->kids[1];
	switch(expr->op)
	{
	case OP_BITOR:
		val.i[0] = expr1->val.i[0] | expr2->val.i[0];
		break;
	case OP_BITAND:
		val.i[0] = expr1->val.i[0] & expr2->val.i[0];
		break;
	case OP_BITXOR:
		val.i[0] = expr1->val.i[0] ^ expr2->val.i[0];
		break;

	case OP_EQUAL:
		EXECUTE_BOP(==);
		break;
	case OP_UNEQUAL:
		EXECUTE_BOP(!=);
		break;
	case OP_MUL:
		EXECUTE_BOP(*);
		break;
	case OP_NEG:
		if (tcode == I4 || tcode == U4) {
			val.i[0] = -expr1->val.i[0];
		}
		break;
	default:
		return expr;
	}
	return Constant(expr->ty, val);
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