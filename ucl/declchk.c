#include "ucl.h"
#include "ast.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"

AstFunction CURRENTF;
FunctionSymbol FSYM;
/**
	function-definition:
		declaration-specifiers(opt)	declarator  declaration-list(opt)	compound-statement
 */
static void CheckDeclarationSpecifiers(AstSpecifiers specs);
static void CheckDeclarator(AstDeclarator dec);

static Type DeriveType(TypeDerivList tyDrvList, Type ty)
{
	while (tyDrvList != NULL) {
		if (tyDrvList->ctor == FUNCTION_RETURN) {
			ty = FunctionReturn(ty, tyDrvList->sig);
		}
		tyDrvList = tyDrvList->next;
	}
	return ty;
}
static void AddParameter(Vector params, char *id, Type ty)
{
	Parameter param;
	ALLOC(param);
	param->id = id;
	param->ty = ty;
	INSERT_ITEM(params, param);
}

static void CheckParameterDeclaration(AstFunctionDeclarator funcDec, AstParameterDeclaration paramDecl)
{
	char *id = NULL;
	Type ty = NULL;
	CheckDeclarationSpecifiers(paramDecl->specs);
	ty = paramDecl->specs->ty;

	CheckDeclarator(paramDecl->dec);
	if (paramDecl->dec->id == NULL && paramDecl->dec->tyDrvList == NULL && ty->categ == VOID && LEN(funcDec->sig->params) == 0)
	{
		return ;
	}
	ty = DeriveType(paramDecl->dec->tyDrvList, ty);
	if (ty != NULL)
		ty = AdjustParameter(ty);

	id = paramDecl->dec->id;
	AddParameter(funcDec->sig->params, id, ty);
}

static void CheckParameterTypeList(AstFunctionDeclarator funcDec)
{
	AstParameterTypeList paramTyList = funcDec->paramTyList;
	AstParameterDeclaration paramDecl;

	paramDecl = (AstParameterDeclaration)paramTyList->paramDecls;
	while (paramDecl) {
		CheckParameterDeclaration(funcDec, paramDecl);
		paramDecl = (AstParameterDeclaration)paramDecl->next;
	}
	funcDec->sig->hasEllipsis = paramTyList->ellipsis;
}

static void CheckFunctionDeclarator(AstFunctionDeclarator dec) 
{
	AstFunctionDeclarator funcDec = (AstFunctionDeclarator)dec;

	CheckDeclarator(funcDec->dec);

	ALLOC(funcDec->sig);
	funcDec->sig->hasProto = funcDec->paramTyList != NULL;
	funcDec->sig->hasEllipsis = 0;
	funcDec->sig->params = CreateVector(4);

	if (funcDec->sig->hasProto)
	{
		CheckParameterTypeList(funcDec);
	}
	ALLOC(funcDec->tyDrvList);
	funcDec->tyDrvList->ctor = FUNCTION_RETURN;
	funcDec->tyDrvList->sig = funcDec->sig;
	funcDec->tyDrvList->next = funcDec->dec->tyDrvList;
	funcDec->id = funcDec->dec->id;
}

static void CheckDeclarator(AstDeclarator dec)
{
	switch(dec->kind) {
		case NK_NameDeclarator:
			break;
		case NK_FunctionDeclarator:
			CheckFunctionDeclarator((AstFunctionDeclarator)dec);
			break;
		default:
			;
	}
}

static void CheckDeclarationSpecifiers(AstSpecifiers specs)
{
	AstToken tok;
	AstNode p;
	Type ty;
	int tyCnt = 0;
	p = specs->tySpecs;
	while (p) {
		tok = (AstToken)p;
		switch(tok->token) {
			case TK_CHAR:
				ty = T(CHAR);
				tyCnt++;
				break;
			case TK_INT:
				ty = T(INT);
				tyCnt++;
				break;
		}
		p = p->next;
	}
	ty = tyCnt == 0 ? T(INT) : ty;
	specs->ty = ty;
	return ;
}


void CheckFunction(AstFunction func)
{
	Symbol sym;
	Type ty;
	int sclass;
	AstNode p;
	Vector params;
	Parameter param;
	CheckDeclarationSpecifiers(func->specs);

	sclass = TK_EXTERN;

	CheckDeclarator(func->dec);
	
	params = func->fdec->sig->params;
	FOR_EACH_ITEM(Parameter, param, params)
		if (param->ty == NULL)
			param->ty = T(INT);
	ENDFOR
	ty = DeriveType(func->dec->tyDrvList, func->specs->ty);

	func->fsym = (FunctionSymbol)AddFunction(func->dec->id, ty, TK_EXTERN);

	CURRENTF = func;
	FSYM = func->fsym;
	{
		Vector v = ((FunctionType)ty)->sig->params;

		FOR_EACH_ITEM(Parameter, param, v)
			AddVariable(param->id, param->ty, TK_AUTO);
		ENDFOR
		FSYM->locals = NULL;
		FSYM->lastv = &FSYM->locals;		
	}
	CheckCompoundStatement(func->stmt);
	// Referencing an undefined label is considered as an error.
}

static void CheckGlobalDeclaraion(AstDeclaration decl)
{
	Symbol sym;
	Type ty;
	CheckDeclarationSpecifiers(decl->specs);
	ty = decl->specs->ty;
	AstDeclarator dec = (AstDeclarator)decl->dec;
	while (dec) {
		CheckDeclarator(dec);
		sym = AddVariable(dec->id, ty);
	}
}

void CheckTranslationUnit(AstTranslationUnit transUnit)
{
	AstNode p;
	Symbol f;
	
	p = transUnit->extDecls;
	/**
		Check every 
		(1)	Function
		(2) GlobalDeclaration
	 */
	while (p)
	{
		if (p->kind == NK_Function)
		{
			CheckFunction((AstFunction)p);
		}
		else
		{
			//assert(p->kind == NK_Declaration);
			CheckGlobalDeclaraion((AstDeclaration)p);
		}
		p = p->next;
	}
}
