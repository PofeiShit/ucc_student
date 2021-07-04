#include "ucl.h"
#include "ast.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"

static void TranslateStatement(AstStatement stmt);
/**
 * This function translates an expression statement
 */
static void TranslateExpressionStatement(AstStatement stmt)
{
	AstExpressionStatement exprStmt = AsExpr(stmt);
	
	if (exprStmt->expr != NULL)
	{
		TranslateExpression(exprStmt->expr);
	}
}

static void TranslateCompoundStatement(AstStatement stmt)
{
	AstCompoundStatement compStmt = AsComp(stmt);
	AstNode p = compStmt->stmts;
	while (p) {
		TranslateStatement((AstStatement)p);
		p = p->next;
	}
}
static void TranslateRetrunStatement(AstStatement stmt)
{
	AstReturnStatement retStmt = AsRet(stmt);
	if (retStmt->expr) {
		GenerateReturn(retStmt->expr->ty, TranslateExpression(retStmt->expr));
	}
	GenerateJump(FSYM->exitBB);
	StartBBlock(CreateBBlock());
}

static void (* StmtTrans[])(AstStatement) = 
{
	TranslateExpressionStatement,
	TranslateRetrunStatement,
	TranslateCompoundStatement,
};

static void TranslateStatement(AstStatement stmt)
{
	(* StmtTrans[stmt->kind - NK_ExpressionStatement])(stmt);
}

//  AST --->IR
static void TranslateFunction(AstFunction func)
{
	BBlock bb;

	FSYM = func->fsym;
	FSYM->entryBB = CreateBBlock();
	FSYM->exitBB = CreateBBlock();
	
	CurrentBB = FSYM->entryBB;
	/**
		When translating statments in entry basic block,
		we will create Basic block when needed and then
		call StartBBlock() to translate the corresponding
		basic blocks .
		However, the exit block is created and translated
		explicitely here.
	 */
	TranslateStatement(func->stmt);
	// 
	StartBBlock(FSYM->exitBB);
	// do some optimizations for every basic block, only at the IR level, not ASM level.

	bb = FSYM->entryBB;
	// function f
	//BB0:
	//BB1:
	// function main
	// BB2:
	// BB3:
	while (bb != NULL)
	{
		// to show the basic blocks more accurately	
		bb->sym = CreateLabel();
		bb = bb->next;
	}
}
//  AST  --> IR
void Translate(AstTranslationUnit transUnit)
{
	AstNode p = transUnit->extDecls;

	while (p)
	{
		if (p->kind == NK_Function && ((AstFunction)p)->stmt)
		{
			TranslateFunction((AstFunction)p);
		}
		p = p->next;
	}
}
