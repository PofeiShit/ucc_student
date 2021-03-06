#ifndef __STMT_H_
#define __STMT_H_
#define AST_STATEMENT_COMMON AST_NODE_COMMON
#define AST_LOOP_STATEMENT_COMMON 	\
	AST_STATEMENT_COMMON 			\
	AstExpression expr;				\
	AstStatement stmt;				\
	BBlock loopBB;					\
	BBlock contBB;					\
	BBlock nextBB;		
struct astStatement
{
	AST_STATEMENT_COMMON
};
typedef struct astLoopStatement
{
	AST_LOOP_STATEMENT_COMMON
} *AstLoopStatement;

typedef struct astForStatement
{
	AST_LOOP_STATEMENT_COMMON
	AstExpression initExpr;
	AstExpression incrExpr;
	BBlock testBB;
} *AstForStatement;

typedef struct astExpressionStatement
{
	AST_STATEMENT_COMMON
	AstExpression expr;
} *AstExpressionStatement;

typedef struct astReturnStatement
{
	AST_STATEMENT_COMMON
	AstExpression expr;
} *AstReturnStatement;

typedef struct astIfStatement
{
	AST_STATEMENT_COMMON
	AstExpression expr;
	AstStatement thenStmt;
	AstStatement elseStmt;
} *AstIfStatement;

typedef struct astBreakStatement
{
	AST_STATEMENT_COMMON
	AstStatement target;
} *AstBreakStatement;

typedef struct astContinueStatement
{
	AST_STATEMENT_COMMON
	AstLoopStatement target;
} *AstContinueStatement;

typedef struct astCaseStatement
{
	AST_STATEMENT_COMMON
	AstExpression expr;
	AstStatement stmt;
	struct astCaseStatement *nextCase;
	BBlock respBB;
} *AstCaseStatement;

typedef struct astDefaultStatement
{
	AST_STATEMENT_COMMON
	AstStatement stmt;
	BBlock respBB;
} *AstDefaultStatement;

typedef struct switchBucket
{
	int ncase;
	int minVal;
	int maxVal;
	AstCaseStatement cases;
	AstCaseStatement *tail;
	struct switchBucket *prev;
} *SwitchBucket;

typedef struct astSwitchStatement
{
	AST_STATEMENT_COMMON
	AstExpression expr;
	AstStatement stmt;
	AstCaseStatement cases;
	AstDefaultStatement defStmt;
	SwitchBucket buckets;
	int nbucket;
	BBlock nextBB;
	BBlock defBB;	
} *AstSwitchStatement;

typedef struct astCompoundStatement
{
	AST_STATEMENT_COMMON
	AstNode decls;
	AstNode stmts;
	//	local variables that has initializer-list.
	Vector ilocals;
} *AstCompoundStatement;
#define AsExpr(stmt) ((AstExpressionStatement)stmt)
#define AsComp(stmt) ((AstCompoundStatement)stmt)
#define AsRet(stmt) ((AstReturnStatement)stmt)
#define AsIf(stmt) ((AstIfStatement)stmt)
#define AsLoop(stmt)   ((AstLoopStatement)stmt)
#define AsFor(stmt) ((AstForStatement)stmt)
#define AsBreak(stmt) ((AstBreakStatement)stmt)
#define AsContinue(stmt) ((AstContinueStatement)stmt)
#define AsSwitch(stmt) ((AstSwitchStatement)stmt)
#define AsDef(stmt) ((AstDefaultStatement)stmt)
#define AsCase(stmt) ((AstCaseStatement)stmt)
AstStatement CheckCompoundStatement(AstStatement stmt);
#endif
