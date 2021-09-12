#include "ucl.h"
#include "ast.h"
#include "expr.h"


static struct tokenOp TokenOps[] = 
{
#define TOKENOP(tok, bop, uop) {bop, uop},
#include "tokenop.h"
#undef  TOKENOP
};

// operators' precedence
static int Prec[] =
{
#define OPINFO(op, prec, name, func, opcode) prec,
#include "opinfo.h"
	0
#undef OPINFO
};

AstExpression Constant0;

/**
 *  primary-expression:
 *		ID
 *		constant
 *		string-literal
 *		( expression )
 */
static AstExpression ParsePrimaryExpression(void)
{
	AstExpression expr;


	switch (CurrentToken)
	{
	case TK_ID:	
		CREATE_AST_NODE(expr, Expression);

		expr->op = OP_ID;
		expr->val = TokenValue;
		NEXT_TOKEN;
		return expr;

	case TK_INTCONST:
		CREATE_AST_NODE(expr, Expression);
		expr->ty = T(INT + CurrentToken - TK_INTCONST);
		expr->op = OP_CONST;
		expr->val = TokenValue;
		NEXT_TOKEN;
		return expr;

	case TK_STRING:			// "ABC"

		CREATE_AST_NODE(expr, Expression);
		expr->ty = ArrayOf(((String)TokenValue.p)->len + 1, T(CHAR));
		expr->op = OP_STR;
		expr->val = TokenValue;
		NEXT_TOKEN;
		return expr;


	default:
		// Error(&TokenCoord, "Expect identifier, string, constant or (");
		return Constant0;
	}
}
static AstExpression ParsePostfixExpression(void)
{
	AstExpression expr, p;
	expr = ParsePrimaryExpression();

	while (1)
	{
		switch (CurrentToken)
		{
		case TK_LPAREN:		// postfix-expression ( [argument-expression-list] )

			CREATE_AST_NODE(p, Expression);

			p->op = OP_CALL;
			p->kids[0] = expr;
			NEXT_TOKEN;
			if (CurrentToken != TK_RPAREN)
			{
				AstNode *tail;
				/// function call expression's second kid is actually
				/// a list of expression instead of a single expression
				p->kids[1] = ParsePrimaryExpression();
				tail = &p->kids[1]->next;
				while (CurrentToken == TK_COMMA) {
					NEXT_TOKEN
					*tail = (AstNode)ParsePrimaryExpression();
					tail = &(*tail)->next;
				}
			}
			Expect(TK_RPAREN);
			expr = p;
			break;

		default:
			return expr;
		}
	}
}
/**
 *  assignment-expression:
 *      conditional-expression
 *      unary-expression assignment-operator assignment-expression
 *  assignment-operator:
 *      = *= /= %= += -= <<= >>= &= ^= |=
 *  There is a little twist here: the parser always treats the first nonterminal
 *  as a conditional expression.
 */
AstExpression ParseAssignmentExpression(void)
{
	AstExpression expr;	 

	expr = ParsePostfixExpression();

	if (CurrentToken >= TK_ASSIGN && CurrentToken <= TK_MOD_ASSIGN) {
		AstExpression asgnExpr;
		CREATE_AST_NODE(asgnExpr, Expression);
		asgnExpr->op = BINARY_OP;
		asgnExpr->kids[0] = expr;
		NEXT_TOKEN;		 
		asgnExpr->kids[1] = ParsePostfixExpression();
		return asgnExpr;
	}
	return expr;
}
/**
 *  expression:
 *      assignment-expression
 *      expression , assignment-expression
 */
AstExpression ParseExpression(void)
{
	AstExpression expr, comaExpr;

	expr = ParseAssignmentExpression();

	while (CurrentToken == TK_COMMA) {
		CREATE_AST_NODE(comaExpr, Expression);
		comaExpr->op = OP_COMMA;
		comaExpr->kids[0] = expr;
		NEXT_TOKEN;		
		comaExpr->kids[1] = ParseAssignmentExpression();
		expr = comaExpr;		
	}
	return expr;
}