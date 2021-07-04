#include "ucl.h"
#include "ast.h"
#include "stmt.h"

static AstStatement ParseExpressionStatement(void)
{
	AstExpressionStatement exprStmt;

	CREATE_AST_NODE(exprStmt, ExpressionStatement);

	if (CurrentToken != TK_SEMICOLON)
	{
		exprStmt->expr = ParseExpression();
	}
	Expect(TK_SEMICOLON);
	return (AstStatement)exprStmt;
}
static AstStatement ParseReturnStatement(void)
{
	AstReturnStatement retStmt;

	CREATE_AST_NODE(retStmt, ReturnStatement);

	NEXT_TOKEN;
	if (CurrentToken != TK_SEMICOLON) {
		retStmt->expr = ParseExpression();
	}
	Expect(TK_SEMICOLON);
	return (AstStatement)retStmt;
}
static AstStatement ParseStatement(void)
{
	switch (CurrentToken)
	{
	case TK_RETURN:
		return ParseReturnStatement();
	default:
		return ParseExpressionStatement();
	}
}

AstStatement ParseCompoundStatement(void)
{
	AstCompoundStatement compStmt;
	AstNode *tail;
	CREATE_AST_NODE(compStmt, CompoundStatement);
	NEXT_TOKEN;
	tail = &compStmt->stmts;
	while (CurrentToken != TK_RBRACE && CurrentToken != TK_END)
	{
		*tail = (AstNode)ParseStatement();
		tail = &(*tail)->next;
		if (CurrentToken == TK_RBRACE)
			break;
	}
	Expect(TK_RBRACE);
	return (AstStatement)compStmt;
}
