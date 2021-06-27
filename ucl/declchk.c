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
void CheckFunctionDeclarator(AstFunctionDeclarator dec) {
	AstFunctionDeclarator funcDec = (AstFunctionDeclarator)dec;
	CheckDeclarator(funcDec->dec);
	ALLOC(funcDec->tyDrvList);
	funcDec->tyDrvList->ctor = FUNCTION_RETURN;
	funcDec->tyDrvList->next = funcDec->dec->tyDrvList;
	funcDec->id = funcDec->dec->id;
}
void CheckDeclarator(AstDeclarator dec)
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

static Type DeriveType(TypeDerivList tyDrvList, Type ty)
{
	while (tyDrvList != NULL) {
		if (tyDrvList->ctor == FUNCTION_RETURN) {
			ty = FunctionReturn(ty);
		}
		tyDrvList = tyDrvList->next;
	}
	return ty;
}

void CheckFunction(AstFunction func)
{
	Symbol sym;
	Type ty;
	AstNode p;
	CheckDeclarationSpecifiers(func->specs);

	CheckDeclarator(func->dec);

	ty = DeriveType(func->dec->tyDrvList, func->specs->ty);

	func->fsym = (FunctionSymbol)AddFunction(func->dec->id, ty);

	CURRENTF = func;
	FSYM = func->fsym;
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
