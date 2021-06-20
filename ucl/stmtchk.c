#include "ucl.h"
#include "ast.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"

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
	return stmt;
}

static AstStatement (*Stmtcheckers[])(AstStatement) = 
{
	CheckExpressionStatment,
	CheckReturnStatement
};

static AstStatement CheckStatement(AstStatement stmt)
{
	return (*Stmtcheckers[stmt->kind - NK_ExpressionStatement])(stmt);
}

AstStatement CheckCompoundStatement(AstStatement stmt)
{
	AstCompoundStatement compStmt = AsComp(stmt);

	AstNode p = compStmt->stmts;
	while(p) 
	{
		CheckStatement((AstStatement)p);
		p = p->next;
	}
	return stmt;

}
