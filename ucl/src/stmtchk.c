#include "ucl.h"
#include "ast.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"

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
static AstStatement CheckLocalStatement(AstStatement stmt)
{
	AstStatement s;
	EnterScope();
	s = CheckCompoundStatement(stmt);
	ExitScope();
	return stmt;
}
static AstStatement (*Stmtcheckers[])(AstStatement) = 
{
	CheckExpressionStatment,
	CheckReturnStatement,
	CheckIfStatement,
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
