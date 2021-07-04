#include "ucl.h"
#include "ast.h"
#include "expr.h"

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
 *  expression:
 *      assignment-expression
 *      expression , assignment-expression
 */
AstExpression ParseExpression(void)
{
	AstExpression expr;
	expr = ParsePostfixExpression();
	return expr;
}
