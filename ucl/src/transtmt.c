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
	Vector ilocals = compStmt->ilocals;
	Symbol v;
	int i;
	for (i = 0; i < LEN(ilocals); i++) {
		InitData initd;
		Type ty;
		Symbol src, dst;
		v = (Symbol)GET_ITEM(ilocals, i);
		initd = AsVar(v)->idata;
		while (initd) {
			ty = initd->expr->ty;
			src = TranslateExpression(initd->expr);
			dst = CreateOffset(ty, v, 0);
			GenerateMove(ty, dst, src);
			initd = initd->next;
		}
	}
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
static void TranslateIfStatement(AstStatement stmt)
{
	AstIfStatement ifStmt = AsIf(stmt);
	BBlock nextBB;
	BBlock trueBB;
	BBlock falseBB;
	nextBB = CreateBBlock();
	trueBB = CreateBBlock();
	if (ifStmt->elseStmt == NULL) {
		TranslateBranch(Not(ifStmt->expr), nextBB, trueBB);
		StartBBlock(trueBB);
		TranslateStatement(ifStmt->thenStmt);
	} else {
		// 
		falseBB = CreateBBlock();
		TranslateBranch(Not(ifStmt->expr), falseBB, trueBB);

		StartBBlock(trueBB);
		TranslateStatement(ifStmt->thenStmt);
		GenerateJump(nextBB);
		
		StartBBlock(falseBB);
		TranslateStatement(ifStmt->elseStmt);
	} 
	StartBBlock(nextBB);
}
// loopBB:
// 		stmt
// continueBB:
//		if (expr) goto loop:
// nextBB
static void TranslateDoStatement(AstStatement stmt)
{
	AstLoopStatement doStmt = AsLoop(stmt);
	doStmt->loopBB = CreateBBlock();
	doStmt->contBB = CreateBBlock();
	doStmt->nextBB = CreateBBlock();
	
	StartBBlock(doStmt->loopBB);
	TranslateStatement(doStmt->stmt);

	StartBBlock(doStmt->contBB);
	TranslateBranch(doStmt->expr, doStmt->loopBB, doStmt->nextBB);

	StartBBlock(doStmt->nextBB);
}
static void TranslateWhileStatement(AstStatement stmt)
{
	AstLoopStatement whileStmt = AsLoop(stmt);
	whileStmt->loopBB = CreateBBlock();
	whileStmt->contBB = CreateBBlock();
	whileStmt->nextBB = CreateBBlock();

	GenerateJump(whileStmt->contBB);

	StartBBlock(whileStmt->loopBB);
	TranslateStatement(whileStmt->stmt);

	StartBBlock(whileStmt->contBB);
	TranslateBranch(whileStmt->expr, whileStmt->loopBB, whileStmt->nextBB);
	
	StartBBlock(whileStmt->nextBB);
}
/*
	expr1
	goto testBB
loopBB:
	stmt
contBB:
	expr3
testBB:
	if expr2 is NULL or expr2 is true goto loopBB
nextBB:
	...
*/
static void TranslateForStatement(AstStatement stmt)
{
	AstForStatement forStmt = AsFor(stmt);
	forStmt->loopBB = CreateBBlock();
	forStmt->contBB = CreateBBlock();
	forStmt->nextBB = CreateBBlock();
	forStmt->testBB = CreateBBlock();

	if (forStmt->initExpr)
		TranslateExpression(forStmt->initExpr);
	GenerateJump(forStmt->testBB);

	StartBBlock(forStmt->loopBB);
	TranslateStatement(forStmt->stmt);
	
	StartBBlock(forStmt->contBB);
	if (forStmt->incrExpr)
		TranslateExpression(forStmt->incrExpr);

	StartBBlock(forStmt->testBB);	
	if (forStmt->expr)
		TranslateBranch(forStmt->expr, forStmt->loopBB, forStmt->nextBB);
	else
		GenerateJump(forStmt->loopBB);

	StartBBlock(forStmt->nextBB);
}
static void TranslateBreakStatement(AstStatement stmt)
{
	AstBreakStatement brkStmt = AsBreak(stmt);
	if (brkStmt->target->kind == NK_SwitchStatement)
		GenerateJump(AsSwitch(brkStmt->target)->nextBB);
	else
		GenerateJump(AsLoop(brkStmt->target)->nextBB);
	StartBBlock(CreateBBlock());
}
static void TranslateContinueStatement(AstStatement stmt)
{
	AstContinueStatement contStmt = AsContinue(stmt);
	GenerateJump(contStmt->target->contBB);
	StartBBlock(CreateBBlock());
}
static void TranslateCaseStatement(AstStatement stmt)
{
	AstCaseStatement caseStmt = AsCase(stmt);
	StartBBlock(caseStmt->respBB);
	TranslateStatement(caseStmt->stmt);
}
static void TranslateDefaultStatement(AstStatement stmt)
{
	AstDefaultStatement defStmt = AsDef(stmt);
	StartBBlock(defStmt->respBB);
	TranslateStatement(defStmt->stmt);
}

static int MergeSwitchBucket(SwitchBucket *pBucket)
{
	SwitchBucket bucket = *pBucket;
	int count = 0;
	while (bucket->prev) {
		if ((bucket->prev->ncase + bucket->ncase + 1) * 2 <= (bucket->maxVal - bucket->minVal))
			break;
		bucket->prev->ncase += bucket->ncase;
		bucket->prev->maxVal = bucket->maxVal;
		*bucket->prev->tail = bucket->cases;
		bucket->prev->tail = bucket->tail;
		bucket = (SwitchBucket)bucket->tail;
		count++;
	}
	*pBucket = bucket;
	return count;
}
static void TranslateSwitchBuckets(SwitchBucket *bucketArray, int left, int right, Symbol choice, BBlock currBB, BBlock defBB)
{
	int mid, len, i;
	AstCaseStatement p;
	BBlock lhalfBB, rhalfBB;
	BBlock *dstBBs;
	Symbol index;
	if (left > right)
		return;
	mid = (left + right) / 2;

	lhalfBB = (left > mid - 1) ? defBB : CreateBBlock();
	rhalfBB = (mid + 1 > right) ? defBB : CreateBBlock();
	len = bucketArray[mid]->maxVal - bucketArray[mid]->minVal + 1;
	dstBBs = (BBlock*)HeapAllocate(CurrentHeap, (len + 1) * sizeof(BBlock));
	for (i = 0; i < len; ++i) {
		dstBBs[i] = defBB;
	}
	dstBBs[len] = NULL;

	p = bucketArray[mid]->cases;
	while (p) {
		i = p->expr->val.i[0] - bucketArray[mid]->minVal;
		dstBBs[i] = p->respBB;
		p = p->nextCase;
	}
	if (currBB != NULL) {
		StartBBlock(currBB);
	}
	if(len == 1 && lhalfBB == rhalfBB){	 	
		GenerateBranch(choice->ty, lhalfBB,JNE, choice, IntConstant(bucketArray[mid]->minVal));
	}else{
		GenerateBranch(choice->ty, lhalfBB, JL, choice, IntConstant(bucketArray[mid]->minVal));
		StartBBlock(CreateBBlock());
		GenerateBranch(choice->ty, rhalfBB, JG, choice, IntConstant(bucketArray[mid]->maxVal));	 
	}
	StartBBlock(CreateBBlock());

	if (len != 1)
	{		
		index = CreateTemp(choice->ty);
		GenerateAssign(choice->ty, index, SUB, choice, IntConstant(bucketArray[mid]->minVal));
		GenerateIndirectJump(dstBBs, len, index);
	}
	else
	{
		GenerateJump(dstBBs[0]);
	}


	StartBBlock(CreateBBlock());

	TranslateSwitchBuckets(bucketArray, left, mid - 1, choice, lhalfBB, defBB);
	TranslateSwitchBuckets(bucketArray, mid + 1, right, choice, rhalfBB, defBB);
}
static void TranslateSwitchStatement(AstStatement stmt)
{
	AstSwitchStatement switchStmt = AsSwitch(stmt);
	AstCaseStatement p, q;
	SwitchBucket bucket, b;
	SwitchBucket *bucketArray;
	int i, val;
	Symbol sym;
	sym = TranslateExpression(switchStmt->expr);
	bucket = b = NULL;
	p = switchStmt->cases;
	while (p) {
		q = p;
		p = p->nextCase;
		q->respBB = CreateBBlock();
		val = q->expr->val.i[0];
		if (bucket && (bucket->ncase + 1) * 2 > (val - bucket->minVal)) {
			bucket->ncase++;
			bucket->maxVal = val;
			*bucket->tail = q;
			bucket->tail = &(q->nextCase);
			switchStmt->nbucket -= MergeSwitchBucket(&bucket);
		} else {
			ALLOC(b);
			b->cases = q;
			b->ncase = 1;
			b->minVal = b->maxVal = val;
			b->tail = &(q->nextCase);
			b->prev = bucket;
			bucket = b;
			switchStmt->nbucket++;
		}
	}
	switchStmt->buckets = bucket;
	bucketArray = (SwitchBucket*)HeapAllocate(CurrentHeap, switchStmt->nbucket * sizeof(SwitchBucket));
	for (i = switchStmt->nbucket - 1; i >= 0; i--) {
		bucketArray[i] = bucket;
		*bucket->tail = NULL;
		bucket = bucket->prev;
	}
	switchStmt->defBB = CreateBBlock();
	if (switchStmt->defStmt) {
		switchStmt->defStmt->respBB = switchStmt->defBB;
		switchStmt->nextBB = CreateBBlock();
	} else {
		switchStmt->nextBB = switchStmt->defBB;
	}
	TranslateSwitchBuckets(bucketArray, 0, switchStmt->nbucket - 1, sym, NULL, switchStmt->defBB);

	if (switchStmt->cases != NULL || switchStmt->defStmt != NULL) {
		TranslateStatement(switchStmt->stmt);
	}
	StartBBlock(switchStmt->nextBB);
}
static void (* StmtTrans[])(AstStatement) = 
{
	TranslateExpressionStatement,
	TranslateCaseStatement,
	TranslateDefaultStatement,
	TranslateSwitchStatement,
	TranslateBreakStatement,
	TranslateContinueStatement,
	TranslateRetrunStatement,
	TranslateIfStatement,
	TranslateDoStatement,
	TranslateWhileStatement,
	TranslateForStatement,
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
	//TempNum = 0;
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
	printf("Translate Done!\n");
}
