#include "ucl.h"
#include "ast.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#define PushStatement(v, stmt) INSERT_ITEM(v, stmt)
#define PopStatement(v) (v->data[--v->len])
#define TopStatement(v) TOP_ITEM(v)
static AstStatement CheckStatement(AstStatement stmt);
static AstStatement CheckExpressionStatment(AstStatement stmt)
{
	AstExpressionStatement exprStmt = AsExpr(stmt);
	if (exprStmt->expr != NULL) {
		exprStmt->expr = CheckExpression(exprStmt->expr);
	}
	return stmt;
}

static AstStatement CheckReturnStatement(AstStatement stmt)
{
	AstReturnStatement retStmt = AsRet(stmt);
	Type rty = FSYM->ty->bty;
		
	CURRENTF->hasReturn = 1;
	if (retStmt->expr)
	{
		retStmt->expr = Adjust(CheckExpression(retStmt->expr), 1);
		// cast to real type
		retStmt->expr = Cast(rty, retStmt->expr);
		return stmt;
	}
		
	return stmt;
}
static AstStatement CheckIfStatement(AstStatement stmt)
{
	AstIfStatement ifStmt = AsIf(stmt);
	ifStmt->expr = Adjust(CheckExpression(ifStmt->expr), 1);
	ifStmt->thenStmt = CheckStatement(ifStmt->thenStmt);
	if (ifStmt->elseStmt) {
		ifStmt->elseStmt = CheckStatement(ifStmt->elseStmt);
	}
	return stmt;
}
static AstStatement CheckLoopStatement(AstStatement stmt)
{
	AstLoopStatement loopStmt = AsLoop(stmt);
	PushStatement(CURRENTF->loops,    stmt);
	PushStatement(CURRENTF->breakable, stmt);

	loopStmt->expr = Adjust(CheckExpression(loopStmt->expr), 1);
	loopStmt->stmt = CheckStatement(loopStmt->stmt);
	PopStatement(CURRENTF->loops);
	PopStatement(CURRENTF->breakable);

	return stmt;
}
static AstStatement CheckForStatement(AstStatement stmt)
{
	AstForStatement forStmt = AsFor(stmt);
	PushStatement(CURRENTF->loops,    stmt);
	PushStatement(CURRENTF->breakable, stmt);

	if (forStmt->initExpr) {
		forStmt->initExpr = CheckExpression(forStmt->initExpr);
	}
	if (forStmt->expr) {
		forStmt->expr = Adjust(CheckExpression(forStmt->expr), 1);
	}
	if (forStmt->incrExpr) {
		forStmt->incrExpr = CheckExpression(forStmt->incrExpr);
	}
	forStmt->stmt = CheckStatement(forStmt->stmt);
	PopStatement(CURRENTF->loops);
	PopStatement(CURRENTF->breakable);

	return stmt;	
}
static AstStatement CheckLocalStatement(AstStatement stmt)
{
	AstStatement s;
	EnterScope();
	s = CheckCompoundStatement(stmt);
	ExitScope();
	return stmt;
}
static AstStatement CheckBreakStatement(AstStatement stmt)
{
	AstBreakStatement brkStmt = AsBreak(stmt);
	brkStmt->target = (AstStatement)TopStatement(CURRENTF->breakable);
	if (brkStmt->target == NULL) {
		Error(NULL, "The break shall appear in a switch or loop");
	}
	return stmt;
}
static AstStatement CheckContinueStatement(AstStatement stmt)
{
	AstContinueStatement contStmt = AsContinue(stmt);
	contStmt->target = (AstLoopStatement)TopStatement(CURRENTF->loops);
	if (contStmt->target == NULL) {
		Error(NULL, "The continue shall appear in a loop");
	}
	return stmt;
}
static AstStatement (*Stmtcheckers[])(AstStatement) = 
{
	CheckExpressionStatment,
	CheckBreakStatement,
	CheckContinueStatement,
	CheckReturnStatement,
	CheckIfStatement,
	CheckLoopStatement, // do {} while();
	CheckLoopStatement, // while() {}
	CheckForStatement,
	CheckLocalStatement,
};

static AstStatement CheckStatement(AstStatement stmt)
{
	return (*Stmtcheckers[stmt->kind - NK_ExpressionStatement])(stmt);
}

AstStatement CheckCompoundStatement(AstStatement stmt)
{
	AstCompoundStatement compStmt = AsComp(stmt);
	AstNode p;
	compStmt->ilocals = CreateVector(1);
	p = compStmt->decls;
	while (p)
	{
		CheckLocalDeclaration((AstDeclaration)p, compStmt->ilocals);
		p = p->next;
	}
	p = compStmt->stmts;
	while(p) 
	{

		CheckStatement((AstStatement)p);
		p = p->next;
	}
	return stmt;

}
