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
			// typedef int INT_ARR[4];
			// INT_ARR test();
			if (ty->categ == ARRAY) {
				Error(NULL, "function cannot return array type");
				ty = PointerTo(ty);
			}
			// typedef int FUNC(void);
			// FUNC test();
			if (ty->categ == FUNCTION) {
				Error(NULL, "function cannot return function type");
				ty = PointerTo(ty);
			}
			ty = FunctionReturn(ty, tyDrvList->sig);
		} else if (tyDrvList->ctor == POINTER_TO) {
			ty = Qualify(tyDrvList->qual, PointerTo(ty));
		} else if (tyDrvList->ctor == ARRAY_OF) {
			// typedef int INT_ARR(void);
			// INT_ARR test[10];
			if (ty->categ == FUNCTION) {
				Error(NULL, "array of function");
			}
			if (IsIncompleteType(ty, !IGNORE_ZERO_SIZE_ARRAY)) {
				Error(NULL, "array has incomplete element type");
				((ArrayType)ty)->len = 1;
			}
			// int a[-1];
			if (tyDrvList->len < 0) {
				Error(NULL, "size of array of negative");
				((ArrayType)ty)->len = 1;
			}
			ty = ArrayOf(tyDrvList->len, ty);
		}
		tyDrvList = tyDrvList->next;
	}
	return ty;
}
static void AddParameter(Vector params, char *id, Type ty)
{
	Parameter param;
	FOR_EACH_ITEM(Parameter, param, params)
		if (param->id && param->id == id) {
			Error(NULL, "Redefine param %s", id);
			return ;
		}
	ENDFOR
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
		// void f(void, int a); || void f(const void) || void f(static void)
		if (paramDecl->next || ty->qual || paramDecl->specs->stgClasses) {
			Error(NULL, "void must be the only paramter");
			paramDecl->next = NULL;
		}
		return ;
	}
	ty = DeriveType(paramDecl->dec->tyDrvList, ty);
	if (ty != NULL)
		ty = AdjustParameter(ty);
	if (ty == NULL) {
		Error(NULL, "Illegal paramter type");
		return ;
	}
	// void f(struct Data d) {}
	if (funcDec->partOfDef && IsIncompleteType(ty, IGNORE_ZERO_SIZE_ARRAY)) {
		Error(NULL, "parameter has incomplete type");
	}
	id = paramDecl->dec->id;
	// void f(int ) {}
	if (id == NULL && funcDec->partOfDef) {
		Error(NULL, "Expect parameter name");
		return ;
	}
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
	funcDec->sig->hasEllipsis = 0;
	funcDec->sig->params = CreateVector(4);

	if (funcDec->paramTyList != NULL)
		CheckParameterTypeList(funcDec);

	ALLOC(funcDec->tyDrvList);
	funcDec->tyDrvList->ctor = FUNCTION_RETURN;
	funcDec->tyDrvList->sig = funcDec->sig;
	funcDec->tyDrvList->next = funcDec->dec->tyDrvList;
	funcDec->id = funcDec->dec->id;
	if (funcDec->partOfDef)
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
			Error(NULL, "The size of the array must be integer constant.");
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
	// typedef int FUNC(void);
	// struct Data {
		// FUNC f;
	// };
	if (fty == NULL || fty->categ == FUNCTION || (fty->size == 0 && (IsIncompleteRecord(fty) || IsIncompleteEnum(fty)))) {
		Error(NULL, "Illegal type");
		return ;
	}
	// struct Buffer {
		// char buf[];
		// int a;
	// }
	if (((RecordType)rty)->hasFlexArray && rty->categ == STRUCT) {
		Error(NULL, "the flexible array must be the last number");
	}
	// struct Buffer {
		// int a;
		// int a;
	// }
	if (id && LookupField(rty, id)) {
		Error(NULL, "member redefinition");
		return ;
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
		or 
		struct Data {
			struct {
				int a;
				int b;
			}
			int c;
		};
	 */	
	if (stDec == NULL) {
		if (IsRecordSpecifier(stDecl->specs->tySpecs)) {
			AstStructSpecifier spec = (AstStructSpecifier) stDecl->specs->tySpecs;
			if (spec->id == NULL) {
				AddField(rty, NULL, stDecl->specs->ty);
				return ;
			}
		}
		Warning(NULL, "declaration does not declara anything");
		return ;
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
			Error(NULL, "Inconsistent tag declaration.");
		}
		return tag->ty;
	}
	else if (stSpec->id == NULL && stSpec->hasLbrace) {
		// struct-or-union	{struct-declaration-list}
		ty = StartRecord(NULL, categ);
		((RecordType)ty)->complete = 1;
		goto chk_decls;
	}
	else if (stSpec->id != NULL && stSpec->hasLbrace) {
		// struct-or-union	id	{struct-declaration-list}
		tag = LookupTag(stSpec->id);
		if (tag == NULL || tag->level < Level) {
			// If it has not been declared yet, or has but in outer-scope.
			ty = StartRecord(stSpec->id, categ);	
			((RecordType)ty)->complete = 1;
			AddTag(stSpec->id, ty);		
		} else if (tag->ty->categ == categ && IsIncompleteRecord(tag->ty)) {
			ty = tag->ty;
			((RecordType)ty)->complete = 1;
		} else {
			if(tag->ty->categ != categ) {
                Error(NULL, "\'%s\'defined as wrong kind of tag.", stSpec->id);
            } else {
				// struct Data {}; 
				// struct Data {};
                Error(NULL, "redefinition of \'%s %s\'.", GetCategName(categ), stSpec->id);
			}
			return tag->ty;
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
static void CheckEnumRedeclaration(AstEnumerator enumer)
{
	Symbol sym;
	sym = LookupID(enumer->id);
	if (sym && sym->level == Level) {
		if(sym->kind != SK_EnumConstant) {
			Error(NULL, "\'%s\'redeclared as different kind of symbol", enumer->id);
		} else {
			Error(NULL, "redeclaration of enumerator \'%s\'", enumer->id);
		}
    }
}
static int CheckEnumerator(AstEnumerator enumer, int last, Type ty)
{
	CheckEnumRedeclaration(enumer);
	if (enumer->expr == NULL) {
		AddEnumConstant(enumer->id, ty, last + 1);
		return last + 1;
	} else {
		enumer->expr = CheckConstantExpression(enumer->expr);
		AddEnumConstant(enumer->id, ty, enumer->expr->val.i[0]);
		return enumer->expr->val.i[0];
	}
}
static Type CheckEnumSpecifier(AstEnumSpecifier enumSpec)
{
	AstEnumerator enumer;
	Symbol tag;
	Type ty;
	int last;
	if (enumSpec->id == NULL && enumSpec->enumers == NULL)
		return T(INT);
	if (enumSpec->id != NULL && enumSpec->enumers == NULL) {
		tag = LookupTag(enumSpec->id);
		if (tag == NULL)
			tag = AddTag(enumSpec->id, Enum(enumSpec->id));
		return tag->ty;
	} else if (enumSpec->id == NULL && enumSpec->enumers != NULL) {
		ty = T(INT);
	} else {
		EnumType ety;
		tag = LookupTag(enumSpec->id);
		if (tag == NULL || tag->level < Level) {
			tag = AddTag(enumSpec->id, Enum(enumSpec->id));
		}
		ety = (EnumType)tag->ty;
		ty = tag->ty;
	}
	enumer = (AstEnumerator)enumSpec->enumers;
	last = -1;
	while (enumer) {
		last = CheckEnumerator(enumer, last, ty);
		enumer = (AstEnumerator)enumer->next;
	}
	return ty;
}
static void CheckDeclarationSpecifiers(AstSpecifiers specs)
{
	AstToken tok;
	AstNode p;
	Type ty;
	int sign = 0, size = 0;
	int tyCnt = 0, signCnt = 0, sizeCnt = 0;
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
		} else if (p->kind == NK_EnumSpecifier) {
			ty = CheckEnumSpecifier((AstEnumSpecifier)p);
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
				case TK_SHORT:
					size = tok->token;
					sizeCnt++;
					break;

				case TK_CHAR:
					ty = T(CHAR);
					tyCnt++;
					break;
				case TK_INT:
					ty = T(INT);
					tyCnt++;
					break;
				case TK_VOID:
					ty = T(VOID);
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
		switch (size) {
			case TK_SHORT:
				ty = T(SHORT + sign);
				break;
			default:
				ty = T(INT + sign);
		}
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

	func->fdec->partOfDef = 1;
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
	CheckDeclarator(tname->dec);
	ty = DeriveType(tname->dec->tyDrvList, tname->specs->ty);

	if (ty == NULL) {
		Error(NULL, "Illegal type");
		ty = T(INT);
	}
	return ty;
}
static AstInitializer CheckInitializerInternal(InitData *tail, AstInitializer init, Type ty, int *offset, int *error)
{
	AstInitializer p;
	int size = 0;
	InitData initd;
	if (IsScalarType(ty)) {
		p = init;
		while (p->lbrace) {
			Warning(NULL, "braces around scalar initializer");
			p = (AstInitializer) p->initials;
		}
		p->expr = Adjust(CheckExpression(p->expr), 1);
		if (!CanAssign(ty, p->expr)) {
			Error(NULL, "Wrong initializer");
			*error = 1;
		} else {
			Type lty = ty, rty = p->expr->ty;
			if ( !((IsPtrType(lty) && IsIntegType(rty) || IsPtrType(rty) && IsIntegType(lty)) && (lty->size == rty->size)) )
				p->expr = Cast(ty, p->expr);
		}
		ALLOC(initd);
		initd->offset = *offset;
		initd->expr = p->expr;
		initd->next = NULL;
		(*tail)->next = initd;
		*tail = initd;
		return (AstInitializer)init->next;
	} else if (ty->categ == ARRAY) {
		int start = *offset;
		p = init->lbrace ? (AstInitializer)init->initials : init;
		// char buf[] = "abcdef" || char buf[] = {"abcdef"};
		if (((init->lbrace && !p->lbrace && p->next == NULL) || !init->lbrace)
			&&  p->expr->op == OP_STR
			&& ty->bty->categ / 2 == p->expr->ty->bty->categ / 2
			&& ty->bty->categ != ARRAY) {
			size = p->expr->ty->size;
			if (ty->size == 0) { // char buf[] = "abcdef";
				ArrayType aty = (ArrayType)ty;
				aty->size = size;
				if (aty->bty->size != 0)
					aty->len = size / aty->bty->size;
			} else if (ty->size == size - p->expr->ty->bty->size) // char buf[6] = "abcdef";
				p->expr->ty->size = size - p->expr->ty->bty->size;
			else if (ty->size < size) { // char buf[3] = "abcdef"
				p->expr->ty->size = ty->size;
				Warning(NULL, "initialize-string for char array is too long");
			}
			ALLOC(initd);
			initd->offset = *offset;
			initd->expr = p->expr;
			initd->next = NULL;
			(*tail)->next = initd;
			*tail = initd;
			return (AstInitializer)init->next;
		}
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
			/*
				struct Data {
					struct {
						int a, b;
					};
					int c;
				} dt = {{20}, 30};
			*/
			if ( (IsRecordType(fld->ty) && fld->id == NULL)) {
				*offset = start;
			}
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
static int CheckCharArrayInit(AstExpression expr, Type bty)
{
	if (expr->op == OP_STR) {
		if ( (bty->categ == CHAR || bty->categ == UCHAR) && expr->ty->bty->categ == CHAR) {
			return 1;
		}
	}
	Error(NULL, "array initializer must be an initializer list");
	return 0;
}
static void CheckInitializer(AstInitializer init, Type ty)
{
	int offset = 0, error = 0;
	struct initData header;
	InitData tail = &header;
	header.next = NULL;

	if (ty->categ == ARRAY && !init->lbrace) {
		if (!CheckCharArrayInit(init->expr, ty->bty)) {
			return ;
		}
	} else if (ty->categ == STRUCT && !init->lbrace) {
		// Data dt1 = dt
		init->expr = Adjust(CheckExpression(init->expr), 1);
		if (! CanAssign(ty, init->expr)) {
			Error(NULL, "Wrong Initialize");
		} else {
			ALLOC(init->idata);
			init->idata->expr = init->expr;
			init->idata->offset = 0;
			init->idata->next = NULL;
		}
		return ;
	}
	CheckInitializerInternal(&tail, init, ty, &offset, &error);
	if (error)
		return ;
	init->idata = header.next;
	// do not support bit
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
	if (expr->op == OP_ADD || expr->op == OP_SUB) {
		// to deal (ptr+n)+k
		addr = CheckAddressConstant(expr->kids[0]);
		if (addr == NULL || expr->kids[1]->op != OP_CONST)
			return NULL;
		expr->kids[0] = addr->kids[0];
		expr->kids[1]->val.i[0] += (expr->op == OP_ADD ? 1 : -1) * addr->kids[1]->val.i[0];
		return expr;
	}
	if (expr->op == OP_ADDRESS) {
		addr = expr->kids[0];
	} else {
		addr = expr;
	}
	while (addr->op == OP_INDEX || addr->op == OP_MEMBER) {
		// arr[3][4] || a.b.c
		if (addr->op == OP_INDEX) {
			if (addr->kids[1]->op != OP_CONST) {
				return NULL;
			}
			offset += addr->kids[1]->val.i[0];
		} else {
			Field fld = (Field)addr->val.p;
			offset += fld->offset;
		}
		addr = addr->kids[0];
	}
	if (addr->op != OP_ID || (expr->op != OP_ADDRESS && ! addr->isarray && ! addr->isfunc))
		return NULL;
	((Symbol)addr->val.p)->ref++;
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
		// global declaration: int number = f();
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
	if (decl->specs->sclass == TK_TYPEDEF) {
		CheckTypedef(decl);
		return ;
	}
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
		if (IsFunctionType(ty)) {
			if ((sym == LookupID(initDec->dec->id)) == NULL) {
				sym = AddFunction(initDec->dec->id, ty, TK_EXTERN);
			}
			goto next;
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
		if (IsFunctionType(ty)) {
			if ((sym = LookupID(initDec->dec->id)) == NULL )
			{
				sym = AddFunction(initDec->dec->id, ty, sclass == 0 ? TK_EXTERN : sclass);
			}
			goto next;
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
		if (! IsCompatibleType(ty, sym->ty)) {
			Error(NULL, "Incompatiable with previous definition", initDec->dec->id);
			goto next;
		}
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
