#ifndef __DECL_H_
#define __DECL_H_

enum { DEC_ABSTRACT=0x01, DEC_CONCRETE=0x02};
enum { POINTER_TO, ARRAY_OF, FUNCTION_RETURN };
#define AST_DECLARATOR_COMMON   \
    AST_NODE_COMMON             \
    struct astDeclarator *dec;  \
    char *id;                   \
	TypeDerivList tyDrvList;

typedef struct tdname
{
	char *id;
	int level;
	int overload;
	int overloadLevel;
} *TDName;

typedef struct astSpecifiers *AstSpecifiers;

typedef struct typeDerivList
{
	int ctor; // type construct 
	union 
	{
		int qual;		// pointer qualifier
		int len;		// array size
		Signature sig; // function signature
	};
	struct typeDerivList *next;

} *TypeDerivList;

typedef struct astDeclarator
{
	AST_DECLARATOR_COMMON
} *AstDeclarator;

typedef struct astTypedefName
{
	AST_NODE_COMMON
	char *id;
	Symbol sym;
} *AstTypedefName;

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
	AstDeclarator dec;
};
typedef struct astPointerDeclarator
{
	AST_DECLARATOR_COMMON
	AstNode tyQuals;
} *AstPointerDeclarator;

typedef struct astEnumerator
{
	AST_NODE_COMMON
	char *id;
	AstExpression expr;
} *AstEnumerator;

typedef struct astEnumSpecifier
{
	AST_NODE_COMMON
	char *id;
	AstNode enumers;
} *AstEnumSpecifier;

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
typedef struct astInitializer
{
	AST_NODE_COMMON
	int lbrace;
	union {
		AstNode initials;
		AstExpression expr; // assignment-expression
	};
	InitData idata;
} *AstInitializer;
typedef struct astInitDeclarator
{
	AST_NODE_COMMON
	AstDeclarator dec;
	AstInitializer init;
} *AstInitDeclarator;

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
	int partOfDef;	// function definition or declaration flag
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
	AstNode initDecs;
};
void CheckLocalDeclaration(AstDeclaration decl, Vector v);
Type CheckTypeName(AstTypeName tname);
extern AstFunction CURRENTF;
#define IsRecordSpecifier(spec) (spec->kind == NK_StructSpecifier)

#endif
