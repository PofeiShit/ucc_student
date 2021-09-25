#include "ucl.h"
#include "grammer.h"
#include "ast.h"
#include "decl.h"

int FIRST_Declaration[] = { FIRST_DECLARATION, 0};

static AstDeclarator ParseDeclarator();
static AstSpecifiers ParseDeclarationSpecifiers(void);

static AstStructDeclaration ParseStructDeclaration(void)
{
	AstStructDeclaration stDecl;
	AstNode *tail;
	CREATE_AST_NODE(stDecl, StructDeclaration);
	// to support 	struct {	;	},	empty struct/union declaration	
	if (CurrentToken == TK_SEMICOLON){		
		NEXT_TOKEN;
		return NULL;
	}	
	stDecl->specs = ParseDeclarationSpecifiers();	
	if (stDecl->specs->stgClasses != NULL)
	{
		//Error(&stDecl->coord, "Struct/union member should not have storage class");
		stDecl->specs->stgClasses = NULL;
	}	
	// if (stDecl->specs->tyQuals == NULL && stDecl->specs->tySpecs == NULL)
	// {
		//Error(&stDecl->coord, "Expect type specifier or qualifier");
	// }	
	// an extension to C89, supports anonymous struct/union member in struct/union
	// struct {
	//		int;
	// }
	if (CurrentToken == TK_SEMICOLON)
	{	
		NEXT_TOKEN;
		return stDecl;		
	}	
	stDecl->stDecs = (AstNode)ParseDeclarator();	
	tail = &stDecl->stDecs->next;	
	while (CurrentToken == TK_COMMA)
	{
		NEXT_TOKEN;
		// TODO: support a:4
		*tail = (AstNode)ParseDeclarator();
		tail = &(*tail)->next;
	}
	// printf("CurrentToken:%d\t%d\n", CurrentToken, TK_SEMICOLON);
	Expect(TK_SEMICOLON);
	
	return stDecl;	
}
static AstStructSpecifier ParseStructOrUnionSpecifier(void)
{
	AstStructSpecifier stSpec;
	AstNode *tail;
	CREATE_AST_NODE(stSpec, StructSpecifier);	
	NEXT_TOKEN;
	switch (CurrentToken) 
	{
		case TK_ID:
			stSpec->id = (char*)TokenValue.p;		
			NEXT_TOKEN;
			if (CurrentToken == TK_LBRACE) 
				goto lbrace;			
			return stSpec;
	
		case TK_LBRACE:
lbrace:
			NEXT_TOKEN;
			stSpec->hasLbrace = 1;

			if (CurrentToken == TK_RBRACE) {
				NEXT_TOKEN;
				return stSpec;
			}
			tail = &stSpec->stDecls;
			while (CurrentTokenIn(FIRST_Declaration)) {				
				*tail = (AstNode)ParseStructDeclaration();
				if (*tail != NULL) 
					tail = &(*tail)->next;
			}
			Expect(TK_RBRACE);	
			return stSpec;

		default:
			return stSpec;
	}
}
static AstSpecifiers ParseDeclarationSpecifiers(void)
{
	AstSpecifiers specs;
	AstToken tok;
	AstNode *scTail, *tsTail;

	CREATE_AST_NODE(specs, Specifiers);
	scTail = &specs->stgClasses;
	tsTail = &specs->tySpecs;	
next_specifiers:
	switch(CurrentToken) 
	{
	case TK_EXTERN:
	case TK_STATIC:
		CREATE_AST_NODE(tok, Token);
		tok->token = CurrentToken;
		*scTail = (AstNode)tok;
		scTail = &tok->next;		
		NEXT_TOKEN;
		break;

	case TK_VOID:
	case TK_CHAR:
	case TK_INT:
		CREATE_AST_NODE(tok, Token);
		tok->token = CurrentToken;
		specs->tySpecs = (AstNode)tok;
		NEXT_TOKEN;
		break;
	case TK_STRUCT:
		*tsTail = (AstNode)ParseStructOrUnionSpecifier();
		tsTail = &(*tsTail)->next;		
		break;

	default:
		return specs;
	}
	goto next_specifiers;
}
/**
 *  direct-declarator:
 *		ID
 *		( declarator )
 *
 *  direct-abstract-declarator:
 *		( abstract-declarator)
 *		nil
 */
static AstDeclarator ParseDirectDeclarator()
{
	AstDeclarator dec;

	CREATE_AST_NODE(dec, NameDeclarator);
	if (CurrentToken == TK_ID)
	{
		dec->id = (char*)TokenValue.p;		
		NEXT_TOKEN;
	}
	return dec;
}

int IsTypeName(int tok)
{
	return tok >= TK_CHAR && tok <= TK_VOID;
}

static AstParameterDeclaration ParseParameterDeclaration(void)
{
	AstParameterDeclaration paramDecl;
	CREATE_AST_NODE(paramDecl, ParameterDeclaration);
	paramDecl->specs = ParseDeclarationSpecifiers();
	paramDecl->dec = ParseDeclarator();
	return paramDecl;
}

AstParameterTypeList ParseParameterTypeList(void)
{
	AstParameterTypeList paramTyList;
	AstNode *tail;
	CREATE_AST_NODE(paramTyList, ParameterTypeList);

	paramTyList->paramDecls = (AstNode)ParseParameterDeclaration();
	tail = &paramTyList->paramDecls->next;
	while (CurrentToken == TK_COMMA) {
		NEXT_TOKEN
		if (CurrentToken == TK_ELLIPSIS) {
			paramTyList->ellipsis = 1;
			NEXT_TOKEN
			break;
		}
		*tail = (AstNode)ParseParameterDeclaration();
		tail = &(*tail)->next;
	}
	return paramTyList;
}

static AstDeclarator ParsePostfixDeclarator()
{
	AstDeclarator dec = ParseDirectDeclarator();
	while (1) {
		if (CurrentToken == TK_LPAREN) {
			AstFunctionDeclarator funcDec;

			CREATE_AST_NODE(funcDec, FunctionDeclarator);
			funcDec->dec = dec;
			NEXT_TOKEN;
			if (IsTypeName(CurrentToken)) {
				funcDec->paramTyList = ParseParameterTypeList();
			}
			Expect(TK_RPAREN);
			dec = (AstDeclarator)funcDec;
		}
		else
		{
			return dec;
		}
	}
}
static AstDeclarator ParseDeclarator()
{
	if (CurrentToken == TK_MUL) {
		// * declarator
		AstPointerDeclarator ptrDec;
		CREATE_AST_NODE(ptrDec, PointerDeclarator);
		NEXT_TOKEN;
		ptrDec->dec = ParseDeclarator();
		return (AstDeclarator)ptrDec;
	}
	return ParsePostfixDeclarator();
}

static AstDeclaration ParseCommonHeader(void)
{
	AstDeclaration decl;
	AstNode *tail;

	CREATE_AST_NODE(decl, Declaration);
	// declaration-specifiers
	decl->specs = ParseDeclarationSpecifiers();
		// f(int a, int b);		
	if (CurrentToken != TK_SEMICOLON) {
		decl->dec = (AstNode)ParseDeclarator();
		tail = &decl->dec->next;
		while (CurrentToken == TK_COMMA) {
			NEXT_TOKEN;
			*tail = (AstNode)ParseDeclarator();
			tail = &(*tail)->next;
		}
	}
	
	return decl;
}

static AstFunctionDeclarator GetFunctionDeclarator(AstDeclarator dec)
{
	if (dec == NULL || dec->next != NULL)
		return NULL;
	while (dec) {
		if (dec->kind == NK_FunctionDeclarator &&
				dec->dec && dec->dec->kind == NK_NameDeclarator) {
			break;
		}
		dec = dec->dec;
	}
	return (AstFunctionDeclarator)dec;
}
/**
declaration
	declaration-specifiers [init-declarator-list]
 */
AstDeclaration ParseDeclaration(void)
{
	AstDeclaration decl;

	decl = ParseCommonHeader();
	Expect(TK_SEMICOLON);

	return decl;
}
/**
 *  external-declaration:
 *		function-definition
 *		declaration
 *
 *  function-definition:
 *		declaration-specifiers declarator [declaration-list] compound-statement
 *
 *  declaration:
 *		declaration-specifiers [init-declarator-list] ;
 *
 *  declaration-list:
 *		declaration
 *		declaration-list declaration
 */
static AstNode ParseExternalDeclaration(void)
{
	AstDeclaration decl = NULL;
	// AstInitDeclarator initDec = NULL;
	AstDeclarator dec = NULL;
	AstFunctionDeclarator fdec;

	decl = ParseCommonHeader();

	// initDec = (AstInitDeclarator)decl->dec;
	dec = (AstDeclarator)decl->dec;
	//printf("%d,%d\n", fdec->kind, fdec->dec->kind);
	fdec = GetFunctionDeclarator(dec);

	//printf("???fdec:%d,%d\n", fdec, dec);
	if (fdec != NULL)
	{
		AstFunction func;

		CREATE_AST_NODE(func, Function);
		// the function declaration's coord and specs are treated as the whole function's.
		func->specs = decl->specs;
		/**
		int ** f(int a, int b);
		 astPointerDeclarator-> astPointerDeclarator ->  astFunctionDeclarator --> astDeclarator
			(dec)									(fdec )
		 */
		func->dec = (AstDeclarator)decl->dec;
		//printf("%s\n", decl->dec->dec->kind);
		func->fdec = fdec;

		if (func->fdec->paramTyList) {
			AstNode p = func->fdec->paramTyList->paramDecls;
			while (p)
			{
				p = p->next;
			}

		}
		func->stmt = ParseCompoundStatement();
		return (AstNode)func;
	}
	Expect(TK_SEMICOLON);
	return (AstNode)decl;
}

/**
 *  translation-unit:
 *		external-declaration
 *		translation-unit external-declaration
 */
AstTranslationUnit ParseTranslationUnit(char *filename)
{
	/**
		TranslationUnit AST Node consists of 
			list of AstDeclaration nodes
	 */
	AstTranslationUnit transUnit;
	AstNode *tail;
	ReadSourceFile(filename);
	// allocate a AST_NODE  and set its kind to NK_TranslationUnit.
	CREATE_AST_NODE(transUnit, TranslationUnit);
	tail = &transUnit->extDecls;
	/**
		The classic scheme of Top-Down recursive parsing is :
			next_token();
			parse();
	 */
	NEXT_TOKEN;
	
	while (CurrentToken != TK_END)
	{
		if (CurrentToken == TK_SEMICOLON) {
			NEXT_TOKEN;
			continue;
		}
		*tail = ParseExternalDeclaration();
		tail = &(*tail)->next;
	}
	CloseSourceFile();
	printf("Parse Done!\n");
	return transUnit;
}
