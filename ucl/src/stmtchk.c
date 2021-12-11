#include "ucl.h"
#include "ast.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#define PushStatement(v, stmt) INSERT_ITEM(v, stmt)
#define PopStatement(v) (v->data[--v->len])
#define TopStatement(v) TOP_ITEM(v)
static AstStatement CheckStatement(AstStatement stmt);
static void AddCase(AstSwitchStatement switchStmt, AstCaseStatement caseStmt)
{
	AstCaseStatement p = switchStmt->cases;
	AstCaseStatement *pprev = &switchStmt->cases;
	int diff;
	while (p) {
		diff = caseStmt->expr->val.i[0] - p->expr->val.i[0];
		if (diff < 0)
			break;
		if (diff > 0) {
			pprev = &p->nextCase;
			p = p->nextCase;
		} else {
			Error(NULL, "Repeated constant in a switch statement");
			return ;
		}
	}
	*pprev = caseStmt;
	caseStmt->nextCase = p;
}
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
static AstStatement CheckLocalCompound(AstStatement stmt)
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
static AstStatement CheckCaseStatement(AstStatement stmt)
{
	AstCaseStatement caseStmt = AsCase(stmt);
	AstSwitchStatement switchStmt;
	switchStmt = (AstSwitchStatement)TopStatement(CURRENTF->switches);
	if (switchStmt == NULL) {
		Error(NULL, "A case shall appear in a switch statement.");
		return stmt;
	}
	caseStmt->expr = CheckConstantExpression(caseStmt->expr);
	if (caseStmt->expr == NULL) {
		Error(NULL, "The case value must be integer constant.");
		return stmt;
	}

	caseStmt->stmt = CheckStatement(caseStmt->stmt);
	caseStmt->expr = FoldCast(switchStmt->expr->ty, caseStmt->expr);
	AddCase(switchStmt, caseStmt);
	return stmt;
}
static AstStatement CheckDefaultStatement(AstStatement stmt)
{
	AstDefaultStatement defStmt = AsDef(stmt);
	AstSwitchStatement switchStmt;
	switchStmt = (AstSwitchStatement)TopStatement(CURRENTF->switches);
	if (switchStmt == NULL) {
		Error(NULL, "A default shall appear in a switch statement.");
		return stmt;
	}
	if (switchStmt->defStmt != NULL) {
		Error(NULL, "There shall be only one default label in a switch statement.");
		return stmt;
	}
	defStmt->stmt = CheckStatement(defStmt->stmt);
	switchStmt->defStmt = defStmt;
	return stmt;
}
static AstStatement CheckSwitchStatement(AstStatement stmt)
{
	AstSwitchStatement switchStmt = AsSwitch(stmt);
	PushStatement(CURRENTF->switches, stmt);
	PushStatement(CURRENTF->breakable, stmt);
	switchStmt->expr = Adjust(CheckExpression(switchStmt->expr), 1);
	if (!IsIntegType(switchStmt->expr->ty)) {
		Error(NULL, "The expression in a switch statement shall be integer type.");
		switchStmt->expr->ty = T(INT);
	}
	if (switchStmt->expr->ty->categ < INT) {
		switchStmt->expr = Cast(T(INT), switchStmt->expr);
	}
	switchStmt->stmt = CheckStatement(switchStmt->stmt);
	PopStatement(CURRENTF->switches);
	PopStatement(CURRENTF->breakable);
	return stmt;
}
static AstStatement (*Stmtcheckers[])(AstStatement) = 
{
	CheckExpressionStatment,
	CheckCaseStatement,
	CheckDefaultStatement,
	CheckSwitchStatement,
	CheckBreakStatement,
	CheckContinueStatement,
	CheckReturnStatement,
	CheckIfStatement,
	CheckLoopStatement, // do {} while();
	CheckLoopStatement, // while() {}
	CheckForStatement,
	CheckLocalCompound,
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
