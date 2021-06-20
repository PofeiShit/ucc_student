#include "ucl.h"
#include "ast.h"
#include "decl.h"
static AstSpecifiers ParseDeclarationSpecifiers(void)
{
	AstSpecifiers specs;
	AstToken tok;
	CREATE_AST_NODE(specs, Specifiers);
    CREATE_AST_NODE(tok, Token);
    tok->token = CurrentToken;
    specs->tySpecs = (AstNode)tok;
    NEXT_TOKEN;
	return specs;
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
		dec->id = TokenValue.p;
		//printf("dec->id :%s ",dec->id);
		NEXT_TOKEN;
	}
	return dec;
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
			dec = (AstDeclarator)funcDec;
			Expect(TK_RPAREN);
		}
		else
		{
			return dec;
		}
	}
}
static AstDeclarator ParseDeclarator()
{
	return ParsePostfixDeclarator();
}

static AstDeclaration ParseCommonHeader(void)
{
	AstDeclaration decl;
	// AstNode *tail;

	CREATE_AST_NODE(decl, Declaration);
	// declaration-specifiers
	decl->specs = ParseDeclarationSpecifiers();
		// f(int a, int b);		
    decl->dec = ParseDeclarator();
	return decl;
}

static AstFunctionDeclarator GetFunctionDeclarator(AstDeclarator dec)
{
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
    //fdec = (AstFunctionDeclarator)decl->dec;
	//printf("%d,%d\n", fdec->kind, fdec->dec->kind);
	fdec = GetFunctionDeclarator(dec);
	//printf("???fdec:%d,%d\n", fdec, CurrentToken);
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
		func->dec = decl->dec;
		//printf("%s\n", decl->dec->dec->kind);
		func->fdec = fdec;
		
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

	return transUnit;
}
