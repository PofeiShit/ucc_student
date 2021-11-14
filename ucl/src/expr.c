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

char *OPNames[] = 
{
	#define OPINFO(op, prec, name, func, opcode) name,
	#include "opinfo.h"
	NULL
	#undef OPINFO
};
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

	case TK_LPAREN:
		NEXT_TOKEN;
		expr = ParseExpression();
		Expect(TK_RPAREN);
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
		case TK_LBRACKET:
			CREATE_AST_NODE(p, Expression);
			p->op = OP_INDEX;
			p->kids[0] = expr;
			NEXT_TOKEN;
			p->kids[1] = ParseExpression();
			Expect(TK_RBRACKET);
			expr = p;
			break;
			
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

		case TK_POINTER:
		case TK_DOT:
			CREATE_AST_NODE(p, Expression);
			p->op = (CurrentToken == TK_DOT) ? OP_MEMBER : OP_PTR_MEMBER;
			p->kids[0] = expr;
			NEXT_TOKEN;
			p->val = TokenValue;
			NEXT_TOKEN;
			expr = p;
			break;

		case TK_DEC:
		case TK_INC:
			CREATE_AST_NODE(p, Expression);
			p->op = (CurrentToken == TK_INC) ? OP_POSTINC : OP_POSTDEC;
			p->kids[0] = expr;
			NEXT_TOKEN;
			expr = p;
			break;
		default:
			return expr;
		}
	}
}
/**
 *  unary-expression:
 *		postfix-expression
 *		unary-operator unary-expression
 *		( type-name ) unary-expression
 *		sizeof unary-expression
 *		sizeof ( type-name )
 *
 *  unary-operator:
 *		++ -- & * + - ! ~
 */
 // The grammar of unary-expression in UCC is a little different from C89.
 // There is no cast-expression in UCC.
AstExpression ParseUnaryExpression()
{
	AstExpression expr;
	int t;
	switch(CurrentToken)
	{
	case TK_DEC:
	case TK_INC:
	case TK_BITAND:
	case TK_MUL:
	case TK_ADD:
	case TK_SUB:
	case TK_COMP:
	case TK_NOT:
		CREATE_AST_NODE(expr, Expression);
		expr->op = UNARY_OP;
		NEXT_TOKEN;
		expr->kids[0] = ParseUnaryExpression();
		return expr;
	case TK_SIZEOF:
		CREATE_AST_NODE(expr, Expression);
		expr->op = OP_SIZEOF;
		NEXT_TOKEN;
		if (CurrentToken == TK_LPAREN) {
			NEXT_TOKEN;
			expr->kids[0] = ParseUnaryExpression();	
			Expect(TK_RPAREN);
		} else {
			expr->kids[0] = ParseUnaryExpression();
		}
		return expr;

	case TK_LPAREN:
		BeginPeekToken();
		t = GetNextToken();
		if (IsTypeName(t)) {
			EndPeekToken();
			CREATE_AST_NODE(expr, Expression);
			expr->op = OP_CAST;
			NEXT_TOKEN;
			expr->kids[0] = (AstExpression)ParseTypeName();
			Expect(TK_RPAREN);
			expr->kids[1] = ParseUnaryExpression();
			return expr;
		} else {
			EndPeekToken();
			return ParsePostfixExpression();
		}
		break;
		
	default:
		return ParsePostfixExpression();
	}
}
AstExpression ParseBinaryExpression(int prec)
{
	AstExpression binExpr;
	AstExpression expr;
	int newPrec;
	expr = ParseUnaryExpression();

	while (IsBinaryOP(CurrentToken) && (newPrec = Prec[BINARY_OP]) >= prec) {
		CREATE_AST_NODE(binExpr, Expression);
		binExpr->op = BINARY_OP;
		binExpr->kids[0] = expr;
		NEXT_TOKEN;
		binExpr->kids[1] = ParseBinaryExpression(newPrec + 1);
		expr = binExpr;
	}
	return expr;
}

/**
 *  conditional-expression:
 *      logical-OR-expression
 *      logical-OR-expression ? expression : conditional-expression
 */
static AstExpression ParseConditionalExpression(void)
{
	AstExpression expr;
	expr = ParseBinaryExpression(Prec[OP_OR]);
	if (CurrentToken == TK_QUESTION) {
		AstExpression condExpr;
		CREATE_AST_NODE(condExpr, Expression);
		condExpr->op = OP_QUESTION;
		condExpr->kids[0] = expr;
		NEXT_TOKEN;
		// use colonExpr for pass llt c++ compile
		AstExpression colonExpr;
		CREATE_AST_NODE(colonExpr, Expression);
		colonExpr->kids[0] = ParseExpression();
		Expect(TK_COLON);
		colonExpr->kids[1] = ParseConditionalExpression();
		condExpr->kids[1] = colonExpr;
		return condExpr;
	}
	return expr;

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

	expr = ParseConditionalExpression();

	if (CurrentToken >= TK_ASSIGN && CurrentToken <= TK_MOD_ASSIGN) {
		AstExpression asgnExpr;
		CREATE_AST_NODE(asgnExpr, Expression);
		asgnExpr->op = BINARY_OP;
		asgnExpr->kids[0] = expr;
		NEXT_TOKEN;		 
		asgnExpr->kids[1] = ParseAssignmentExpression();
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

AstExpression ParseConstantExpression(void)
{
	return ParseConditionalExpression();
}