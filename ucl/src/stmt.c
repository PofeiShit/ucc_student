#include "ucl.h"
#include "grammer.h"
#include "ast.h"
#include "stmt.h"

static AstStatement ParseStatement(void);
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
static AstStatement ParseDoStatement(void)
{
	AstLoopStatement doStmt;
	CREATE_AST_NODE(doStmt, DoStatement);
	NEXT_TOKEN;
	doStmt->stmt = ParseStatement();
	Expect(TK_WHILE);
	Expect(TK_LPAREN);
	doStmt->expr = ParseExpression();
	Expect(TK_RPAREN);
	Expect(TK_SEMICOLON);
	return (AstStatement)doStmt;
}
static AstStatement ParseWhileStatement(void)
{
	AstLoopStatement whileStmt;
	CREATE_AST_NODE(whileStmt, WhileStatement);
	NEXT_TOKEN;
	Expect(TK_LPAREN);
	whileStmt->expr = ParseExpression();
	Expect(TK_RPAREN);
	whileStmt->stmt = ParseStatement();

	return (AstStatement)whileStmt;
}
static AstStatement ParseIfStatement(void)
{
	AstIfStatement ifStmt;

	CREATE_AST_NODE(ifStmt, IfStatement);
	NEXT_TOKEN;
	Expect(TK_LPAREN);
	ifStmt->expr = ParseExpression();
	Expect(TK_RPAREN);
	ifStmt->thenStmt = ParseStatement();
	if (CurrentToken == TK_ELSE) {
		NEXT_TOKEN;
		ifStmt->elseStmt = ParseStatement();
	}
	return (AstStatement)ifStmt;
}
static AstStatement ParseForStatement(void)
{
	AstForStatement forStmt;
	CREATE_AST_NODE(forStmt, ForStatement);
	NEXT_TOKEN;
	Expect(TK_LPAREN);
	if (CurrentToken != TK_SEMICOLON)
		forStmt->initExpr = ParseExpression();
	Expect(TK_SEMICOLON);
	if (CurrentToken != TK_SEMICOLON)
		forStmt->expr = ParseExpression();
	Expect(TK_SEMICOLON);
	if (CurrentToken != TK_RPAREN)
		forStmt->incrExpr = ParseExpression();
	Expect(TK_RPAREN);
	forStmt->stmt = ParseStatement();
	return (AstStatement)forStmt;
}
static AstStatement ParseBreakStatement(void)
{
	AstBreakStatement brkStmt;
	CREATE_AST_NODE(brkStmt, BreakStatement);
	NEXT_TOKEN;
	Expect(TK_SEMICOLON);
	return (AstStatement)brkStmt;
}
static AstStatement ParseContinueStatement(void)
{
	AstContinueStatement contStmt;
	CREATE_AST_NODE(contStmt, ContinueStatement);
	NEXT_TOKEN;
	Expect(TK_SEMICOLON);
	return (AstStatement)contStmt;
}
static AstStatement ParseCaseStatement(void)
{
	AstCaseStatement caseStmt;
	CREATE_AST_NODE(caseStmt, CaseStatement);
	NEXT_TOKEN;
	caseStmt->expr = ParseConstantExpression();
	Expect(TK_COLON);
	caseStmt->stmt = ParseStatement();

	return (AstStatement)caseStmt;
}
static AstStatement ParseDefaultStatement(void)
{
	AstDefaultStatement defStmt;
	CREATE_AST_NODE(defStmt, DefaultStatement);
	NEXT_TOKEN;
	Expect(TK_COLON);
	defStmt->stmt = ParseStatement();
	return (AstStatement)defStmt;
}
static AstStatement ParseSwitchStatement(void)
{
	AstSwitchStatement switchStmt;
	CREATE_AST_NODE(switchStmt, SwitchStatement);
	NEXT_TOKEN;
	Expect(TK_LPAREN);
	switchStmt->expr = ParseExpression();
	Expect(TK_RPAREN);
	switchStmt->stmt = ParseStatement();
	return (AstStatement)switchStmt;
}
AstStatement ParseCompoundStatement(void)
{
	AstCompoundStatement compStmt;
	AstNode *tail;

	Level++;
	CREATE_AST_NODE(compStmt, CompoundStatement);
	NEXT_TOKEN;
	tail = &compStmt->decls;

	while (CurrentTokenIn(FIRST_Declaration))
	{
		// for example, 	"f(20,30);",  f is an id;  but f(20,30) is a statement, not declaration.
		if (CurrentToken == TK_ID && !IsTypeName(CurrentToken))
			break;

		*tail = (AstNode)ParseDeclaration();
		tail = &(*tail)->next;
	}

	tail = &compStmt->stmts;
	while (CurrentToken != TK_RBRACE && CurrentToken != TK_END)
	{		
		*tail = (AstNode)ParseStatement();
		tail = &(*tail)->next;
		if (CurrentToken == TK_RBRACE)
			break;
	}
	Expect(TK_RBRACE);
	PostCheckTypedef();
	Level--;
		
	return (AstStatement)compStmt;
}
static AstStatement ParseStatement(void)
{
	switch (CurrentToken)
	{
	case TK_CASE:
		return ParseCaseStatement();
	case TK_DEFAULT:
		return ParseDefaultStatement();
	case TK_SWITCH:
		return ParseSwitchStatement();
	case TK_CONTINUE:
		return ParseContinueStatement();
	case TK_BREAK:
		return ParseBreakStatement();
	case TK_FOR:
		return ParseForStatement();
	case TK_DO:
		return ParseDoStatement();
	case TK_WHILE:
		return ParseWhileStatement();
	case TK_IF:
		return ParseIfStatement();
	case TK_RETURN:
		return ParseReturnStatement();
	case TK_LBRACE:
		return ParseCompoundStatement();
	default:
		return ParseExpressionStatement();
	}
}
