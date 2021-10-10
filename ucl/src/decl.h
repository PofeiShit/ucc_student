#ifndef __DECL_H_
#define __DECL_H_

enum { POINTER_TO, ARRAY_OF, FUNCTION_RETURN };
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
		int len;		// array size
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
	AstNode stgClasses;
	AstNode tyQuals;
	// After semantics check ,we know the storage-class
	int sclass;	
	Type ty;
};
struct astTypeName
{
	AST_NODE_COMMON;
	AstSpecifiers specs;
};
typedef struct astPointerDeclarator
{
	AST_DECLARATOR_COMMON
} *AstPointerDeclarator;

typedef struct astStructSpecifier
{
	AST_NODE_COMMON
	char *id;
	// struct-declaration-list,
	// see function ParseStructOrUnionSpecifier()
	AstNode stDecls;
	int hasLbrace;
} *AstStructSpecifier;

typedef struct astStructDeclarator
{
	AST_NODE_COMMON
	AstDeclarator dec;
	AstExpression expr;
} *AstStructDeclarator;

typedef struct astStructDeclaration
{
	AST_NODE_COMMON
	AstSpecifiers specs;
	AstNode stDecs;
} *AstStructDeclaration;

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

typedef struct astArrayDeclarator
{
	AST_DECLARATOR_COMMON
	AstExpression expr;
} *AstArrayDeclarator;

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
	Vector loops;
	Vector switches;
	Vector breakable;
	int hasReturn;
} *AstFunction;

struct astDeclaration
{
	AST_NODE_COMMON
	// declaration-specifiers:	(static | int | const | ...) +
	AstSpecifiers specs;
	AstNode dec;
};
void CheckLocalDeclaration(AstDeclaration decl, Vector v);
Type CheckTypeName(AstTypeName tname);
extern AstFunction CURRENTF;
#endif
