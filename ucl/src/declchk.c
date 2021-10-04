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
		} else if (tyDrvList->ctor == POINTER_TO) {
			ty = PointerTo(ty);
		} else if (tyDrvList->ctor == ARRAY_OF) {
			ty = ArrayOf(tyDrvList->len, ty);
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
	EnterParameterList();
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
	SaveParameterListTable();	
	LeaveParemeterList();
}
static void CheckPointerDeclarator(AstPointerDeclarator dec)
{
	AstPointerDeclarator ptrDec = (AstPointerDeclarator)dec;
	CheckDeclarator(ptrDec->dec);
	ALLOC(ptrDec->tyDrvList);
	ptrDec->tyDrvList->ctor = POINTER_TO;
	ptrDec->tyDrvList->next = ptrDec->dec->tyDrvList;
	ptrDec->id = ptrDec->dec->id;

}
static void CheckArrayDeclarator(AstArrayDeclarator arrDec)
{
	CheckDeclarator(arrDec->dec);
	if (arrDec->expr) {
		if ((arrDec->expr = CheckConstantExpression(arrDec->expr)) == NULL) {
			;
		}
	}
	ALLOC(arrDec->tyDrvList);
	arrDec->tyDrvList->ctor = ARRAY_OF;
	arrDec->tyDrvList->len = arrDec->expr ? arrDec->expr->val.i[0] : 0;
	arrDec->tyDrvList->next = arrDec->dec->tyDrvList;
	arrDec->id = arrDec->dec->id;
}
static void CheckDeclarator(AstDeclarator dec)
{
	switch(dec->kind) {
		case NK_NameDeclarator:
			break;
		case NK_ArrayDeclarator:
			CheckArrayDeclarator((AstArrayDeclarator)dec);
			break;
		case NK_FunctionDeclarator:
			CheckFunctionDeclarator((AstFunctionDeclarator)dec);
			break;
		case NK_PointerDeclarator:
			CheckPointerDeclarator((AstPointerDeclarator)dec);
			break;
		default:
			;
	}
}
static void CheckStructDeclarator(Type rty, AstDeclarator stDec, Type fty)
{
	char *id = NULL;
	if (stDec != NULL)
	{
		CheckDeclarator(stDec);
		id = stDec->id;		
		fty = DeriveType(stDec->tyDrvList, fty);
	}
	AddField(rty, id, fty);	
}
static void CheckStructDeclaration(AstStructDeclaration stDecl, Type rty)
{
	AstDeclarator stDec;
	CheckDeclarationSpecifiers(stDecl->specs);
	stDec = (AstDeclarator)stDecl->stDecs;	
	/**
		struct Data{
			int;		----->		anonymous struct-declaration	
			......
		}
	 */	
	if (stDec == NULL) {
		;
	}
	/**
		struct Data{
			int c, d;			
		}
	*/
	while (stDec)
	{
		CheckStructDeclarator(rty, stDec, stDecl->specs->ty);
		stDec = (AstDeclarator)stDec->next;
	}  

}
static Type CheckStructOrUnionSpecifier(AstStructSpecifier stSpec)
{
	int categ = STRUCT;
	Symbol tag;
	Type ty;
	AstStructDeclaration stDecl;

	if (stSpec->id != NULL && !stSpec->hasLbrace) {
		// struct-or-union id		
		tag = LookupTag(stSpec->id);
		if (tag == NULL) {
			ty = StartRecord(stSpec->id, categ);
			tag = AddTag(stSpec->id, ty);
		} 
		else if (tag->ty->categ != categ)
		{
			//Error(&stSpec->coord, "Inconsistent tag declaration.");
			;
		}
		return tag->ty;
	}
	else if (stSpec->id == NULL && stSpec->hasLbrace) {
		// struct-or-union	{struct-declaration-list}
		ty = StartRecord(NULL, categ);
		goto chk_decls;
	}
	else if (stSpec->id != NULL && stSpec->hasLbrace) {
		// struct-or-union	id	{struct-declaration-list}
		tag = LookupTag(stSpec->id);
		if (tag == NULL || tag->level < Level) {
			// If it has not been declared yet, or has but in outer-scope.
			ty = StartRecord(stSpec->id, categ);	
			AddTag(stSpec->id, ty);		
		}
		goto chk_decls;		
	}
	else {
		// struct-or-union;		illegal & already alarmed during syntax parsing	
		ty = StartRecord(NULL, categ);
		EndRecord(ty);
		return ty;		
	}
chk_decls:
	stDecl = (AstStructDeclaration)stSpec->stDecls;
	while (stDecl)
	{
		CheckStructDeclaration(stDecl, ty);
		stDecl = (AstStructDeclaration)stDecl->next;
	}
	// calculate the size of the struct and other type-informations.
	EndRecord(ty);
	return ty;	
}

static void CheckDeclarationSpecifiers(AstSpecifiers specs)
{
	AstToken tok;
	AstNode p;
	Type ty;
	int tyCnt = 0;
	//storage-class-specifier:		extern	, auto,	static, register, ... 
	tok = (AstToken)specs->stgClasses;
	if (tok) {
		specs->sclass = tok->token;
	}
	p = specs->tySpecs;
	while (p) {
		if (p->kind == NK_StructSpecifier)
		{
			ty = CheckStructOrUnionSpecifier((AstStructSpecifier)p);
			tyCnt++;
		} else {
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
	//The default storage-class of function definition is extern.
	if ((sclass = func->specs->sclass) == 0)
	{
		sclass = TK_EXTERN;
	}
	CheckDeclarator(func->dec);
	
	params = func->fdec->sig->params;
	FOR_EACH_ITEM(Parameter, param, params)
		if (param->ty == NULL)
			param->ty = T(INT);
	ENDFOR
	ty = DeriveType(func->dec->tyDrvList, func->specs->ty);

	func->fsym = (FunctionSymbol)AddFunction(func->dec->id, ty, sclass);

	CURRENTF = func;
	FSYM = func->fsym;
	RestoreParameterListTable();	
	{
		Vector v = ((FunctionType)ty)->sig->params;

		FOR_EACH_ITEM(Parameter, param, v)
			AddVariable(param->id, param->ty, TK_AUTO);
		ENDFOR
		FSYM->locals = NULL;
		FSYM->lastv = &FSYM->locals;		
	}
	CheckCompoundStatement(func->stmt);
	ExitScope();	
	// Referencing an undefined label is considered as an error.
}
//
Type CheckTypeName(AstTypeName tname)
{
	Type ty;
	CheckDeclarationSpecifiers(tname->specs);
	ty = tname->specs->ty;
	return ty;
}
void CheckLocalDeclaration(AstDeclaration decl, Vector v)
{
	Symbol sym;
	Type ty;
	int sclass;	
	CheckDeclarationSpecifiers(decl->specs);
	ty = decl->specs->ty;
	sclass = decl->specs->sclass;
	if (sclass == 0)
	{
		sclass = TK_AUTO;
	}	
	// check declarator
	AstDeclarator dec = (AstDeclarator)decl->dec;
	while (dec) {
		CheckDeclarator(dec);

		ty = DeriveType(dec->tyDrvList, decl->specs->ty);	
		if (ty == NULL)
		{
			// Error(&initDec->coord, "Illegal type");
			ty = T(INT);
		}				
		if ((sym = LookupID(dec->id)) == NULL || sym->level < Level)
		{
			VariableSymbol vsym;
			vsym = (VariableSymbol)AddVariable(dec->id, ty, sclass);
		}
		// sclass = sclass == TK_EXTERN ? sym->sclass : sclass;	
		// if (sym->sclass == TK_EXTERN){
		// 	sym->sclass = sclass;			
		// }		
		dec = (AstDeclarator)dec->next;	
	}
}

static void CheckGlobalDeclaration(AstDeclaration decl)
{	
	Symbol sym;
	Type ty;
	int sclass;	
	CheckDeclarationSpecifiers(decl->specs);
	ty = decl->specs->ty;
	sclass = decl->specs->sclass;
	// check declarator
	AstDeclarator dec = (AstDeclarator)decl->dec;
	while (dec) {
		CheckDeclarator(dec);

		ty = DeriveType(dec->tyDrvList, decl->specs->ty);	
		if (ty == NULL)
		{
			// Error(&initDec->coord, "Illegal type");
			ty = T(INT);
		}				
		if ((sym = LookupID(dec->id)) == NULL)
		{
			sym = AddVariable(dec->id, ty, sclass);
		}
		// sclass = sclass == TK_EXTERN ? sym->sclass : sclass;	
		// if (sym->sclass == TK_EXTERN){
		// 	sym->sclass = sclass;			
		// }		
		dec = (AstDeclarator)dec->next;	
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
			CheckGlobalDeclaration((AstDeclaration)p);
		}
		p = p->next;
	}
	printf("Check done\n");
}
