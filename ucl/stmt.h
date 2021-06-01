#ifndef __STMT_H_
#define __STMT_H_
#define AST_STATEMENT_COMMON AST_NODE_COMMON

struct astStatement
{
	AST_STATEMENT_COMMON
};
typedef struct astExpressionStatement
{
	AST_STATEMENT_COMMON
	AstExpression expr;
} *AstExpressionStatement;

typedef struct astCompoundStatement
{
	AST_STATEMENT_COMMON
	// AstNode decls;
	AstNode stmts;
	//	local variables that has initializer-list.
	// Vector ilocals;
} *AstCompoundStatement;
#define AsExpr(stmt) ((AstExpressionStatement)stmt)
#define AsComp(stmt) ((AstCompoundStatement)stmt)
AstStatement CheckCompoundStatement(AstStatement stmt);
#endif
