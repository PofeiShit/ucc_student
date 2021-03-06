#include "ucl.h"
#include "grammer.h"
#include "ast.h"
#include "decl.h"
static Vector TypedefNames, OverloadNames;

int FIRST_Declaration[] = { FIRST_DECLARATION, 0};

static AstDeclarator ParseDeclarator(int kind);
static AstSpecifiers ParseDeclarationSpecifiers(void);

static char* GetOutermostID(AstDeclarator dec)
{
	if (dec->kind == NK_NameDeclarator) 
		return dec->id;
	return GetOutermostID(dec->dec);
}
static int IsTypedefName(char *id)
{
	Vector v = TypedefNames;
	TDName tn;
	FOR_EACH_ITEM(TDName, tn, v)
		if (tn->id == id && tn->level <= Level && ! tn->overload)
			return 1;
	ENDFOR
	return 0;
}
static void CheckTypedefName(int sclass, char *id)
{
	Vector v;
	TDName tn;
	if (id == NULL)
		return;
	v = TypedefNames;
	if (sclass == TK_TYPEDEF) {
		FOR_EACH_ITEM(TDName, tn, v)
			if (tn->id == id) {
				if (Level < tn->level)
					tn->level = Level;
				return;
			}
		ENDFOR
		ALLOC(tn);
		tn->id = id;
		tn->level = Level;
		tn->overload = 0;
		INSERT_ITEM(v, tn);
	} else {
		FOR_EACH_ITEM(TDName, tn, v)
			if (tn->id == id && Level > tn->level) {
				tn->overload = 1;
				tn->overloadLevel = Level;
				INSERT_ITEM(OverloadNames, tn);
				return ;
			}
		ENDFOR
	}
}
static void PreCheckTypedef(AstDeclaration decl)
{
	AstNode p;
	int sclass = 0;
	if (decl->specs->stgClasses != NULL) {
		sclass = ((AstToken)decl->specs->stgClasses)->token;
	}
	p = decl->initDecs;
	while (p != NULL) {
		CheckTypedefName(sclass, GetOutermostID(((AstInitDeclarator)p)->dec));
		p = p->next;
	}
}
void PostCheckTypedef(void)
{
	TDName tn;
	int overloadCount = 0;

	FOR_EACH_ITEM(TDName, tn, OverloadNames)
		if (Level <= (tn->overloadLevel)) {
			tn->overload = 0;
		} else if (tn->overload != 0) {
			overloadCount++;
		}
	ENDFOR
	if (overloadCount == 0)
		OverloadNames->len = 0;
}
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
		Error(NULL, "Struct/union member should not have storage class");
		stDecl->specs->stgClasses = NULL;
	}	
	if (stDecl->specs->tyQuals == NULL && stDecl->specs->tySpecs == NULL)
	{
		Error(NULL, "Expect type specifier or qualifier");
	}	
	// an extension to C89, supports anonymous struct/union member in struct/union
	// struct {
	//		int;
	// }
	if (CurrentToken == TK_SEMICOLON)
	{	
		NEXT_TOKEN;
		return stDecl;		
	}	
	stDecl->stDecs = (AstNode)ParseDeclarator(DEC_CONCRETE);	
	tail = &stDecl->stDecs->next;	
	while (CurrentToken == TK_COMMA)
	{
		NEXT_TOKEN;
		*tail = (AstNode)ParseDeclarator(DEC_CONCRETE);
		tail = &(*tail)->next;
	}
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
			// case: struct ;
			Error(NULL, "Expect identifier or { after struct/union");
			return stSpec;
	}
}
static AstEnumerator ParseEnumerators()
{
	AstEnumerator enumer;
	CREATE_AST_NODE(enumer, Enumerator);
	if (CurrentToken != TK_ID) {
		Error(NULL, "The enumeration constant must be identifer");
		return enumer;
	}
	enumer->id = (char*)TokenValue.p;
	NEXT_TOKEN;
	if (CurrentToken == TK_ASSIGN) {
		NEXT_TOKEN;
		enumer->expr = ParseConstantExpression();
	}
	return enumer;
}	
static AstEnumSpecifier ParseEnumSpecifiers()
{
	AstEnumSpecifier enumSpec;
	AstNode *tail;
	CREATE_AST_NODE(enumSpec, EnumSpecifier);
	NEXT_TOKEN;
	if (CurrentToken == TK_ID) {
		enumSpec->id = (char*)TokenValue.p;
		NEXT_TOKEN;
		if (CurrentToken == TK_LBRACE) 
			goto enumerator_list;
	} else if (CurrentToken == TK_LBRACE) {
enumerator_list:
		NEXT_TOKEN;
		if (CurrentToken == TK_RBRACE) {
			NEXT_TOKEN;
			Error(NULL, "Expect identifiers before '}' token");
			return enumSpec;
		}
		enumSpec->enumers = (AstNode)ParseEnumerators();
		tail = &enumSpec->enumers->next;
		while (CurrentToken == TK_COMMA) {
			NEXT_TOKEN;
			if (CurrentToken == TK_RBRACE)
				break;
			*tail = (AstNode)ParseEnumerators();
			tail = &(*tail)->next;
		}
		Expect(TK_RBRACE);
	}
	else {
		Error(NULL, "Expect identifers or { after enum");
	}
	return enumSpec;
}
static AstSpecifiers ParseDeclarationSpecifiers(void)
{
	AstSpecifiers specs;
	AstToken tok;
	AstNode *scTail, *tsTail, *tqTail;
	int seeTy = 0;
	CREATE_AST_NODE(specs, Specifiers);
	scTail = &specs->stgClasses;
	tsTail = &specs->tySpecs;	
	tqTail = &specs->tyQuals;
next_specifiers:
	switch(CurrentToken) 
	{
	case TK_EXTERN:
	case TK_AUTO:
	case TK_STATIC:
	case TK_TYPEDEF:
		// storage classes
		CREATE_AST_NODE(tok, Token);
		tok->token = CurrentToken;
		*scTail = (AstNode)tok;
		scTail = &tok->next;		
		NEXT_TOKEN;
		break;

	case TK_VOID:
	case TK_CHAR:
	case TK_INT:
	case TK_SHORT:
	case TK_UNSIGNED:
	case TK_SIGNED:
		CREATE_AST_NODE(tok, Token);
		tok->token = CurrentToken;
		*tsTail = (AstNode)tok;
		tsTail = &tok->next;
		seeTy = 1;
		NEXT_TOKEN;
		break;
	case TK_ID:
		if (!seeTy && IsTypedefName((char*)TokenValue.p)) {
			AstTypedefName tname;
			CREATE_AST_NODE(tname, TypedefName);
			tname->id = (char*)TokenValue.p;
			*tsTail = (AstNode)tname;
			tsTail = &tname->next;
			NEXT_TOKEN;
			seeTy = 1;
			break;
		}
		return specs;

	case TK_STRUCT:
		*tsTail = (AstNode)ParseStructOrUnionSpecifier();
		tsTail = &(*tsTail)->next;	
		seeTy = 1;	
		break;
	case TK_CONST:
	case TK_VOLATILE:
		// type qualifiers
		CREATE_AST_NODE(tok, Token);
		tok->token = CurrentToken;
		*tqTail = (AstNode)tok;
		tqTail = &tok->next;
		NEXT_TOKEN;
		break;
	case TK_ENUM:
		*tsTail = (AstNode)ParseEnumSpecifiers();
		tsTail = &(*tsTail)->next;
		seeTy = 1;
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
static AstDeclarator ParseDirectDeclarator(int kind)
{
	AstDeclarator dec;
	if (CurrentToken == TK_LPAREN) {
		int t;
		BeginPeekToken();
		t = GetNextToken();
		if (t != TK_ID && t != TK_LPAREN && t != TK_MUL) {
			EndPeekToken();
			CREATE_AST_NODE(dec, NameDeclarator);
		} else {
			EndPeekToken();
			NEXT_TOKEN;
			dec = ParseDeclarator(kind);
			Expect(TK_RPAREN);
		}
		return dec;
	}
	CREATE_AST_NODE(dec, NameDeclarator);
	if (CurrentToken == TK_ID)
	{
		if (kind == DEC_ABSTRACT) {
			// case: a = (char b)c;
			Error(NULL, "Identifier is not permitted in the abstract declarator");
		}
		dec->id = (char*)TokenValue.p;  
		NEXT_TOKEN;
	} else if (kind == DEC_CONCRETE) {
		Error(NULL, "Expect identifier");
	}
	return dec;
}

int IsTypeName(int tok)
{
	return tok == TK_ID ? IsTypedefName((char*)TokenValue.p) : (tok >= TK_AUTO && tok <= TK_VOID);
}
AstTypeName ParseTypeName(void)
{
	AstTypeName tyName;
	CREATE_AST_NODE(tyName, TypeName);
	tyName->specs = ParseDeclarationSpecifiers();
	tyName->dec = (AstDeclarator)ParseDeclarator(DEC_ABSTRACT);
	return tyName;
}
static AstParameterDeclaration ParseParameterDeclaration(void)
{
	AstParameterDeclaration paramDecl;
	CREATE_AST_NODE(paramDecl, ParameterDeclaration);
	paramDecl->specs = ParseDeclarationSpecifiers();
	paramDecl->dec = (AstDeclarator)ParseDeclarator(DEC_CONCRETE | DEC_ABSTRACT);
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

static AstDeclarator ParsePostfixDeclarator(int kind)
{
	AstDeclarator dec = ParseDirectDeclarator(kind);
	while (1) {
		if (CurrentToken == TK_LPAREN) {
			AstFunctionDeclarator funcDec;

			CREATE_AST_NODE(funcDec, FunctionDeclarator);
			funcDec->dec = dec;
			NEXT_TOKEN;
			if (IsTypeName(CurrentToken)) {
				funcDec->paramTyList = ParseParameterTypeList();
			}
			// do not support old style c
			Expect(TK_RPAREN);
			dec = (AstDeclarator)funcDec;
		}
		else if (CurrentToken == TK_LBRACKET) {
			AstArrayDeclarator arrDec;
			CREATE_AST_NODE(arrDec, ArrayDeclarator);
			arrDec->dec = dec;
			NEXT_TOKEN;
			if (CurrentToken != TK_RBRACKET) {
				arrDec->expr = ParseConstantExpression();
			}
			Expect(TK_RBRACKET);
			dec = (AstDeclarator)arrDec;
		}
		else
		{
			return dec;
		}
	}
}
static AstDeclarator ParseDeclarator(int kind)
{
	if (CurrentToken == TK_MUL) {
		// * declarator
		AstPointerDeclarator ptrDec;
		AstToken tok;
		AstNode *tail;
		CREATE_AST_NODE(ptrDec, PointerDeclarator);
		tail = &ptrDec->tyQuals;
		NEXT_TOKEN;
		while (CurrentToken == TK_CONST) {
			CREATE_AST_NODE(tok, Token);
			tok->token = CurrentToken;
			*tail = (AstNode)tok;
			tail = &tok->next;
			NEXT_TOKEN;
		}
		ptrDec->dec = ParseDeclarator(kind);
		return (AstDeclarator)ptrDec;
	}
	return ParsePostfixDeclarator(kind);
}
static AstInitializer ParseInitializer()
{
	AstInitializer init;
	AstNode *tail;

	CREATE_AST_NODE(init, Initializer);
	if (CurrentToken == TK_LBRACE) {
		init->lbrace = 1;
		NEXT_TOKEN;
		init->initials = (AstNode)ParseInitializer();
		tail = &init->initials->next;
		while (CurrentToken == TK_COMMA) {
			NEXT_TOKEN;
			if (CurrentToken == TK_RBRACE) 
				break;
			*tail = (AstNode)ParseInitializer();
			tail = &(*tail)->next;
		}
		Expect(TK_RBRACE);
	} else {
		init->lbrace = 0;
		init->expr = ParseAssignmentExpression();
	}
	return init;
}
static AstInitDeclarator ParseInitDeclarator()
{
	AstInitDeclarator initDec;
	CREATE_AST_NODE(initDec, InitDeclarator);
	initDec->dec = ParseDeclarator(DEC_CONCRETE);
	if (CurrentToken == TK_ASSIGN) {
		NEXT_TOKEN;
		initDec->init = ParseInitializer();
	}
	return initDec;
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
		decl->initDecs = (AstNode)ParseInitDeclarator();
		tail = &decl->initDecs->next;
		while (CurrentToken == TK_COMMA) {
			NEXT_TOKEN;
			*tail = (AstNode)ParseInitDeclarator();
			tail = &(*tail)->next;
		}
	}

	return decl;
}

static AstFunctionDeclarator GetFunctionDeclarator(AstInitDeclarator initDec)
{
	AstDeclarator dec;

	if (initDec == NULL || initDec->next != NULL || initDec->init != NULL)
		return NULL;
	dec = initDec->dec;
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
	PreCheckTypedef(decl);
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
	AstInitDeclarator initDec = NULL;
	AstFunctionDeclarator fdec;

	decl = ParseCommonHeader();
	if (decl->specs->stgClasses != NULL && ((AstToken)decl->specs->stgClasses)->token == TK_TYPEDEF)
		goto not_func;
	initDec = (AstInitDeclarator)decl->initDecs;
	fdec = GetFunctionDeclarator(initDec);
	if (fdec != NULL)
	{
		AstFunction func;
		if (CurrentToken == TK_SEMICOLON) {
			NEXT_TOKEN;
			if (CurrentToken != TK_LBRACE) 
				return (AstNode)decl;
			Error(NULL, "redundant ;");
		} else if (fdec->paramTyList && CurrentToken != TK_LBRACE) {
			// function declaration but loses ;
			goto not_func;
		}
		CREATE_AST_NODE(func, Function);
		// the function declaration's coord and specs are treated as the whole function's.
		func->specs = decl->specs;
		/**
		int ** f(int a, int b);
		 astPointerDeclarator-> astPointerDeclarator ->  astFunctionDeclarator --> astDeclarator
			(dec)									(fdec )
		 */
		func->dec = (AstDeclarator)initDec->dec;
		func->fdec = fdec;
		Level++;
		if (func->fdec->paramTyList) {
			AstNode p = func->fdec->paramTyList->paramDecls;
			while (p)
			{
				CheckTypedefName(0, GetOutermostID(((AstParameterDeclaration)p)->dec));
				p = p->next;
			}

		}
		// do not support old style c
		Level--;
		func->stmt = ParseCompoundStatement();
		return (AstNode)func;
	}
not_func:
	if (!decl->specs->stgClasses && !decl->specs->tyQuals && !decl->specs->tySpecs) {
		Warning(NULL, "declaratioin specifiers missing, defaulting to int");	
	}
	Expect(TK_SEMICOLON);
	PreCheckTypedef(decl);
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
	TypedefNames = CreateVector(8);
	OverloadNames = CreateVector(8); 
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
