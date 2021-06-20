#include "ucl.h"
#include "ast.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"

FunctionSymbol FSYM;
/**
	function-definition:
		declaration-specifiers(opt)	declarator  declaration-list(opt)	compound-statement
 */
void CheckFunctionDeclarator(AstFunctionDeclarator dec)
{
	AstFunctionDeclarator funcDec = (AstFunctionDeclarator)dec;
	CheckDeclarator(funcDec->dec);
	funcDec->id = funcDec->dec->id;
}
void CheckDeclarator(AstDeclarator dec)
{
	switch(dec->kind) {
		case NK_NameDeclarator:
			break;
		case NK_FunctionDeclarator:
			CheckFunctionDeclarator((AstFunctionDeclarator)dec);
		default:
			;
	}
}

void CheckFunction(AstFunction func)
{
	Symbol sym;
	CheckDeclarator(func->dec);
	func->fsym = (FunctionSymbol)AddFunction(func->dec->id);

	//CURRENTF = func;
	FSYM = func->fsym;
	CheckCompoundStatement(func->stmt);
	// Referencing an undefined label is considered as an error.
}

static void CheckGlobalDeclaraion(AstDeclaration decl)
{
	Symbol sym;
	AstDeclarator dec = (AstDeclarator)decl->dec;
	CheckDeclarator(dec);
	sym = AddVariable(dec->id);
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
