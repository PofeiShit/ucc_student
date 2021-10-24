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
			ty = Qualify(tyDrvList->qual, PointerTo(ty));
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
	int qual = 0;
	AstPointerDeclarator ptrDec = (AstPointerDeclarator)dec;
	AstToken tok = (AstToken)ptrDec->tyQuals;
	CheckDeclarator(ptrDec->dec);
	while (tok) {
		qual |= tok->token == TK_CONST ? CONST : VOLATILE;
		tok = (AstToken)tok->next;
	}
	ALLOC(ptrDec->tyDrvList);
	ptrDec->tyDrvList->ctor = POINTER_TO;
	ptrDec->tyDrvList->qual = qual;
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
	int sign = 0;
	int tyCnt = 0, signCnt = 0;
	int qual = 0;
	//storage-class-specifier:		extern	, auto,	static, register, ... 
	tok = (AstToken)specs->stgClasses;
	if (tok) {
		specs->sclass = tok->token;
	}
	// type qualifiers
	tok = (AstToken)specs->tyQuals;
	while (tok) {
		qual |= (tok->token == TK_CONST ? CONST : CONST);
		tok = (AstToken)tok->next;
	}
	p = specs->tySpecs;
	while (p) {
		if (p->kind == NK_StructSpecifier)
		{
			ty = CheckStructOrUnionSpecifier((AstStructSpecifier)p);
			tyCnt++;
		} else if (p->kind == NK_TypedefName) {
			Symbol sym = LookupID(((AstTypedefName)p)->id);
			if (sym) {
				ty = sym->ty;
			} else {
				ty = T(INT);
			}
			tyCnt++;
		} else {
			tok = (AstToken)p;
			switch(tok->token) {
				case TK_SIGNED:
				case TK_UNSIGNED:
					sign = tok->token;
					signCnt++;
					break;

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
	if (signCnt > 1)
		goto err;

	if (ty == T(CHAR)) {
		sign = (sign == TK_UNSIGNED);
		ty = T(CHAR + sign);
		sign = 0;
	} 
	if (ty == T(INT)) {
		sign = (sign == TK_UNSIGNED);
		ty = T(INT + sign);
	} else if (sign != 0)
		goto err;

	specs->ty = Qualify(qual, ty);
	return ;
err:
	Error(NULL, "Illegal type specifier.");
	specs->ty = T(INT);
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

	func->loops = CreateVector(4);
	func->breakable = CreateVector(4);
	func->switches = CreateVector(4);
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
static void CheckTypedef(AstDeclaration decl)
{
	AstInitDeclarator initDec;
	Type ty;
	Symbol sym;
	initDec = (AstInitDeclarator)decl->initDecs;
	while (initDec) {
		CheckDeclarator(initDec->dec);
		if (initDec->dec->id == NULL)
			goto next;
		ty = DeriveType(initDec->dec->tyDrvList, decl->specs->ty);
		sym = LookupID(initDec->dec->id);
		AddTypedefName(initDec->dec->id, ty);
next:
		initDec = (AstInitDeclarator)initDec->next;
	}
}
//
Type CheckTypeName(AstTypeName tname)
{
	Type ty;
	CheckDeclarationSpecifiers(tname->specs);
	ty = tname->specs->ty;
	return ty;
}
static AstInitializer CheckInitializerInternal(InitData *tail, AstInitializer init, Type ty, int *offset, int *error)
{
	AstInitializer p;
	int size = 0;
	InitData initd;
	if (IsScalarType(ty)) {
		p = init;
		p->expr = Adjust(CheckExpression(p->expr), 1);
		ALLOC(initd);
		initd->offset = *offset;
		initd->expr = p->expr;
		initd->next = NULL;
		(*tail)->next = initd;
		*tail = initd;
		return (AstInitializer)init->next;
	} else if (ty->categ == ARRAY) {
		int start = *offset;
		// printf("Check%d-%d-%d\n", init->lbrace, start, ty->categ);
		p = init->lbrace ? (AstInitializer)init->initials : init;
		// TODO char buf[] = "abcdef"
		while (p) {
			p = CheckInitializerInternal(tail, p, ty->bty, offset, error);
			size += ty->bty->size;
			*offset = start + size;
			if (ty->size == size)
				break;
		}
		if (ty->size == 0) {
			// calculate the len of array int arr[] = {1, 2, 3};
			ArrayType aty = (ArrayType)ty;
			if (aty->bty->size != 0 && aty->len == 0) {
				aty->len = size / aty->bty->size;
			}
			ty->size = size;
		}
		// printf("size:%d\n", ty->size);
		if (init->lbrace) {
			if (p) { // int arr2[3] = {1, 2, 3, 4};
				Warning(NULL, "excess elements in array initializer");
			}
			return (AstInitializer)init->next;
		}
		return p;
	} else if (ty->categ == STRUCT) {
		int start = *offset;
		Field fld = ((RecordType)ty)->flds;
		p = init->lbrace ? (AstInitializer)init->initials : init;

		while (fld && p) {
			*offset = start + fld->offset;

			/* TODO: struct
			if ( (IsRecordType(fld->ty) && fld->id == NULL)) {
				*offset = start;
			}
			*/
			// printf("*offset = %d , start = %d , fld->offset = %d\n",*offset, start, fld->offset);
			p = CheckInitializerInternal(tail, p, fld->ty, offset, error);
			fld = fld->next;
		}
		*offset = start + ty->size;
		if (init->lbrace) {
			if (p != NULL) {
				Warning(NULL, "excess elements in struct initializer");
			}
			return (AstInitializer)init->next;
		}
		return (AstInitializer)p;
	}
	return init;
}
static void CheckInitializer(AstInitializer init, Type ty)
{
	int offset = 0, error = 0;
	struct initData header;
	InitData tail = &header;
	header.next = NULL;

	if (ty->categ == ARRAY && !init->lbrace) {
		// TODO:char arr[] = "abcdfe";
		// if (!CheckArrayInit(init->expr, ty->bty)) {
		// 	return ;
		// }
		return ;
	}
	CheckInitializerInternal(&tail, init, ty, &offset, &error);
	if (error)
		return ;
	init->idata = header.next;
}
/**
 * Check if the initializer expression is address constant.
 * e.g. 
 * int a;
 * int b = &a;
 */
static AstExpression CheckAddressConstant(AstExpression expr)
{
	AstExpression addr;
	AstExpression p;
	int offset = 0;
	if (! IsPtrType(expr->ty))
		return NULL;
	if (expr->op == OP_ADDRESS) {
		addr = expr->kids[0];
	}
	CREATE_AST_NODE(p, Expression);
	p->op = OP_ADD;
	p->ty = expr->ty;
	p->kids[0] = addr;
	{
		union value val;
		val.i[0] = offset;
		val.i[1] = 0;
		p->kids[1] = Constant(T(INT), val);
	}
	return p;
}
static void CheckInitConstant(AstInitializer init)
{
	InitData initd = init->idata;
	while (initd) {
		if (!(initd->expr->op == OP_CONST || initd->expr->op == OP_STR || 
			(initd->expr = CheckAddressConstant(initd->expr)))) {
			Error(NULL, "Initializer must be constant");
		}
		initd = initd->next;
	}
	return ;
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
	AstInitDeclarator initDec = (AstInitDeclarator)decl->initDecs;
	while (initDec) {
		CheckDeclarator(initDec->dec);
		if (initDec->dec->id == NULL)
			goto next;
		ty = DeriveType(initDec->dec->tyDrvList, decl->specs->ty);	
		if (ty == NULL)
		{
			// Error(&initDec->coord, "Illegal type");
			ty = T(INT);
		}				
		if ((sym = LookupID(initDec->dec->id)) == NULL || sym->level < Level)
		{
			VariableSymbol vsym;
			vsym = (VariableSymbol)AddVariable(initDec->dec->id, ty, sclass);
			if (initDec->init) {
				CheckInitializer(initDec->init, ty);
				if (sclass == TK_STATIC) {
					CheckInitConstant(initDec->init);
				} else {
					INSERT_ITEM(v, vsym);
				}
				vsym->idata = initDec->init->idata;
			}
		}
		// sclass = sclass == TK_EXTERN ? sym->sclass : sclass;	
		// if (sym->sclass == TK_EXTERN){
		// 	sym->sclass = sclass;			
		// }	
next:	
		initDec = (AstInitDeclarator)initDec->next;	
	}
}

static void CheckGlobalDeclaration(AstDeclaration decl)
{	
	Symbol sym;
	Type ty;
	int sclass;	
	CheckDeclarationSpecifiers(decl->specs);
	if (decl->specs->sclass == TK_TYPEDEF)
	{
		CheckTypedef(decl);
		return ;
	}
	ty = decl->specs->ty;
	sclass = decl->specs->sclass;
	// check declarator
	AstInitDeclarator initDec = (AstInitDeclarator)decl->initDecs;
	while (initDec) {
		CheckDeclarator(initDec->dec);
		if (initDec->dec->id == NULL)
			goto next;

		ty = DeriveType(initDec->dec->tyDrvList, decl->specs->ty);	
		if (ty == NULL)
		{
			Error(NULL, "Illegal type");
			ty = T(INT);
		}				
		if ((sym = LookupID(initDec->dec->id)) == NULL)
		{
			sym = AddVariable(initDec->dec->id, ty, sclass);
		}
		if (initDec->init) {
			CheckInitializer(initDec->init, ty);
			CheckInitConstant(initDec->init);
			AsVar(sym)->idata = initDec->init->idata;
		}
		// sclass = sclass == TK_EXTERN ? sym->sclass : sclass;	
		// if (sym->sclass == TK_EXTERN){
		// 	sym->sclass = sclass;			
		// }
next:		
		initDec = (AstInitDeclarator)initDec->next;	
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
