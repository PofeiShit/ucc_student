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
AstStatement CheckCompoundStatement(AstStatement stmt);
#endif
