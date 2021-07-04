#ifndef __DECL_H_
#define __DECL_H_

enum { FUNCTION_RETURN };
#define AST_DECLARATOR_COMMON   \
    AST_NODE_COMMON             \
    struct astDeclarator *dec;  \
    char *id;                   \
	TypeDerivList tyDrvList;

typedef struct astSpecifiers *AstSpecifiers;

typedef struct typeDerivList
{
	int ctor; // type construct 
	union 
	{
		Signature sig; // function signature
	};
	struct typeDerivList *next;

} *TypeDerivList;

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
	Type ty;
};

struct astTranslationUnit
{
	AST_NODE_COMMON
	/**
		ExternalDeclarations
	 */
	AstNode extDecls;
};

typedef struct astParameterDeclaration
{
	AST_NODE_COMMON
	AstSpecifiers specs;
	AstDeclarator dec;
} *AstParameterDeclaration;

typedef struct astParameterTypeList
{
	AST_NODE_COMMON
	AstNode paramDecls;
	int ellipsis;
} *AstParameterTypeList;

typedef struct astFunctionDeclarator
{
	AST_DECLARATOR_COMMON
	AstParameterTypeList paramTyList;
	Signature sig;
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
	int hasReturn;
} *AstFunction;

struct astDeclaration
{
	AST_NODE_COMMON
	// declaration-specifiers:	(staic | int | const | ...) +
	AstSpecifiers specs;
	AstNode dec;
};

extern AstFunction CURRENTF;
#endif
