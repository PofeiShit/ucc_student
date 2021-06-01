#ifndef __DECL_H_
#define __DECL_H_


#define AST_DECLARATOR_COMMON   \
    AST_NODE_COMMON             \
    struct astDeclarator *dec;  \
    char *id;                   \

typedef struct astSpecifiers *AstSpecifiers;

typedef struct astDeclarator
{
	AST_DECLARATOR_COMMON
} *AstDeclarator;

typedef struct astToken
{
	AST_NODE_COMMON
	int token;
} *AstToken;

struct astSpecifiers
{
	AST_NODE_COMMON
	AstNode tySpecs;	
};

struct astTranslationUnit
{
	AST_NODE_COMMON
	/**
		ExternalDeclarations
	 */
	AstNode extDecls;
};

typedef struct astFunctionDeclarator
{
	AST_DECLARATOR_COMMON
} *AstFunctionDeclarator;

typedef struct astFunction
{
	AST_NODE_COMMON
	// declaration-specifiers(opt)
	AstSpecifiers specs;
	AstDeclarator dec;
	AstFunctionDeclarator fdec;

	// compound-statement
	AstStatement stmt;
	FunctionSymbol fsym;
} *AstFunction;

struct astDeclaration
{
	AST_NODE_COMMON
	// declaration-specifiers:	(staic | int | const | ...) +
	AstSpecifiers specs;
	AstNode dec;
};

#endif
